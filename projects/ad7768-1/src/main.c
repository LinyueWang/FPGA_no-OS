/***************************************************************************//**
* @file ad77681_evb.c
* @brief Implementation of Main Function.
* @author SPopa (stefan.popa@analog.com)
********************************************************************************
* Copyright 2020(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
* - Neither the name of Analog Devices, Inc. nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
* - The use of this software may or may not infringe the patent rights
* of one or more patent holders. This license does not release you
* from the requirement that you obtain separate licenses from these
* patent holders to use this software.
* - Use of the software either in source or binary form, must be run
* on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <sleep.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef XILINX_PLATFORM
#include <xil_cache.h>
#include <xparameters.h>
#endif

#include "ad7768-1.h"
#include "gpio_extra.h"
#include "spi_engine.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

//#define SPI_ENGINE_OFFLOAD_ENABLED
#define NO_SAMPLES 2
//#define USE_CRC

#define AD77681_DMA_BASEADDR		XPAR_AXI_AD77681_DMA_BASEADDR
#define AD77681_SPI_ENGINE_BASEADDR	XPAR_SPI_ADC_AXI_REGMAP_BASEADDR
#define AD77681_SPI_CS			0
#define AD77681_DDR_RX			XPAR_DDR_MEM_BASEADDR + 0x800000
#define AD77681_DDR_TX			AD77681_DDR_RX + 9800000

#define AD77681_GPIO_RESET		86
#define AD77681_GPIO_SYNC_IN		85
#define AD77681_GPIO_DRDY		91

static struct ad77681_init_param init_params = {
	.spi_init_param = &(struct spi_init_param){
		.max_speed_hz = 1000000, /* 20 MHz */
		.chip_select = AD77681_SPI_CS,
		.mode = SPI_MODE_3,
		.extra = &(struct spi_engine_init_param){
			.type = SPI_ENGINE,
			.axi_clk_hz = 40000000,
			.spi_engine_baseaddr = AD77681_SPI_ENGINE_BASEADDR
		}
	},
	.gpio_init_drdy = &(struct gpio_init_param){
		.number = AD77681_GPIO_DRDY,
		.extra = &(struct xil_gpio_init_param){
			.type = GPIO_PS,
			.device_id = XPAR_XGPIOPS_0_DEVICE_ID
		}
	},
	.gpio_init_reset = &(struct gpio_init_param){
		.number = AD77681_GPIO_RESET,
		.extra = &(struct xil_gpio_init_param){
			.type = GPIO_PS,
			.device_id = XPAR_XGPIOPS_0_DEVICE_ID
		}
	},
	.power_mode = AD77681_FAST,
	.mclk_div = AD77681_MCLK_DIV_2,
	.conv_mode = AD77681_CONV_ONE_SHOT,
	.diag_mux_sel = AD77681_POSITIVE_FS,
	.conv_diag_sel = false,
	.conv_len = AD77681_CONV_24BIT,
#ifdef USE_CRC
	.crc_sel = AD77681_CRC,
#else
	.crc_sel = AD77681_NO_CRC,
#endif
	.status_bit = 0
};

int main()
{
	struct ad77681_dev	*adc_dev;
#ifdef XILINX_PLATFORM
	Xil_ICacheEnable();
	Xil_DCacheEnable();
#endif
	ad77681_setup(&adc_dev, init_params);
#ifndef SPI_ENGINE_OFFLOAD_ENABLED
	uint8_t		adc_data[3];
	uint8_t		i;
	uint32_t	sample_counter;

	for(sample_counter = 0; sample_counter < NO_SAMPLES; sample_counter++)
	{
		ad77681_spi_read_adc_data(adc_dev, adc_data);

		printf("\r[ADC DATA]: 0x");
		for(i = 0; i < sizeof(adc_data) / sizeof(uint8_t); i++)
			printf("%02X", adc_data[i]);
	}
#else
	uint32_t tx_buf[] = {AD77681_REG_READ(AD77681_REG_ADC_DATA)};
	uint32_t *data;
	uint32_t i;

	ad77681_set_conv_mode(adc_dev,
			      AD77681_CONV_CONTINUOUS,
			      adc_dev->diag_mux_sel,
			      adc_dev->conv_diag_sel);

	struct spi_message *m = NULL;
	/* Write 1 byte (read adc data command) */
	struct spi_transfer t = {
		.tx_buff = (void *)tx_buf,
		.num_bits = 8,
		.length = 1,
		.cs = SPI_CS_LOW
	};
	spi_message_init(&m, &t);
	/* Read 3 bytes (raw data) */
	t = (struct spi_transfer){
		.rx_buff = (void *)AD77681_DDR_RX,
		.length = 1,
#ifdef USE_CRC
		.num_bits = 32,
#else
		.num_bits = 24,
#endif
	};
	spi_message_add_transfer(m, &t);
	t = (struct spi_transfer){
		.cs = SPI_CS_HIGH
	};
	spi_message_add_transfer(m, &t);
	m->is_memory_mapped = true;
	m->rx_dma_baseaddr = (void *)AD77681_DMA_BASEADDR,
	m->tx_dma_baseaddr = NULL,
	spi_message_exec(adc_dev->spi_desc, m, NO_SAMPLES);

	/* Do stuff while the ADC is sampling */
	usleep(10000); 
#ifdef XILINX_PLATFORM
	Xil_DCacheDisable();
	Xil_ICacheDisable();
#endif
	/* Print the raw data */
	//data = (uint32_t *)msg.rx_addr;
	data = (uint32_t *)AD77681_DDR_RX;
	for(i = 0; i < NO_SAMPLES; i++)
	{
#ifdef USE_CRC
		printf("\r\nSample[%03d] : 0x%02X", i + 1, data[i] >> 8);
		printf(" CRC = 0x%02X",data[i] & 0xFF);
#else
		printf("\r\nSample[%03d] : 0x%02X", i + 1, data[i]);
#endif
	}
#endif
	spi_engine_remove(adc_dev->spi_desc);
	printf("\r\nExiting program");
	
	return 0;
}
