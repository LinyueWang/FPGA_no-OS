/*******************************************************************************
 *   @file   spi_engine.h
 *   @brief  Header file of SPI Engine core features.
 *   @author Sergiu Cuciurean (sergiu.cuciurean@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
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

#ifndef SPI_ENGINE_H
#define SPI_ENGINE_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "spi_extra.h"
#include "spi_engine_private.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define OFFLOAD_DISABLED		0x00
#define OFFLOAD_TX_EN			BIT(0)
#define OFFLOAD_RX_EN			BIT(1)
#define OFFLOAD_TX_RX_EN		OFFLOAD_TX_EN | OFFLOAD_RX_EN

#define SPI_ENGINE_MSG_QUEUE_END	0xFFFFFFFF

/* Spi engine commands */
#define	WRITE(no_bytes)			((SPI_ENGINE_INST_TRANSFER << 12) |\
	(SPI_ENGINE_INSTRUCTION_TRANSFER_W << 8) | no_bytes)

#define	READ(no_bytes)			((SPI_ENGINE_INST_TRANSFER << 12) |\
	(SPI_ENGINE_INSTRUCTION_TRANSFER_R << 8) | no_bytes)

#define	WRITE_READ(no_bytes)		((SPI_ENGINE_INST_TRANSFER << 12) |\
	(SPI_ENGINE_INSTRUCTION_TRANSFER_RW << 8) | no_bytes)

#define SLEEP(time)			SPI_ENGINE_CMD_SLEEP(time & 0xF)

/* The delay and chip select parmeters are passed over the engine descriptor
and will be used inside the function call */
#define CS_HIGH				SPI_ENGINE_CMD_ASSERT(0x03, 0xFF)
#define CS_LOW				SPI_ENGINE_CMD_ASSERT(0x03, 0x00)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @struct spi_engine_init_param
 * @brief  Structure containing the init parameters needed by the SPI engine
 */
struct spi_engine_init_param {
	/** Type of implementation */
	enum xil_spi_type	type;
	/** Axi clock of the SPI Engine core */
	uint32_t		axi_clk_hz;
	/** Base address where the HDL core is situated */
	uint32_t 		spi_engine_baseaddr;
};

/**
 * @struct spi_engine_desc
 * @brief  Structure representing an SPI engine device
 */
struct spi_engine_desc {
	/** Type of implementation */
	enum xil_spi_type	type;
	/** Pointer to a DMAC used in transmission */
	struct axi_dmac		*offload_tx_dma;
	/** Pointer to a DMAC used in reception */
	struct axi_dmac		*offload_rx_dma;
	/** Axi clock of the SPI Engine core */
	uint32_t		axi_clk_hz;
	/** Offload's module transfer direction : TX, RX or both */
	uint8_t			offload_config;
	/** Base address where the HDL core is situated */
	uint32_t		spi_engine_baseaddr;
	/** Clock divider used in transmission delays */
	uint32_t		clk_div;
	/** Data with of one SPI transfer ( in bits ) */
	uint8_t			data_width;
	/** The maximum data width supported by the engine */
	uint8_t 		max_data_width;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize the SPI engine device */
int32_t spi_engine_init(struct spi_desc *desc,
			const struct spi_init_param *param);

int32_t spi_engine_message_exec(struct spi_desc *desc,
				struct spi_message *msg,
				uint32_t no_msg);
				
/* Write and read data over SPI using the SPI engine */
int32_t spi_engine_write_and_read(struct spi_desc *desc,
				  uint8_t *data,
				  uint8_t bytes_number);

/* Free the resources used by the SPI engine device */
int32_t spi_engine_remove(struct spi_desc *desc);

#endif // SPI_ENGINE_H
