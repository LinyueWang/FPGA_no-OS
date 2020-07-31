/***************************************************************************//**
 *   @file   no_os_hal.c
 *   @brief  No-OS Hardware Abstraction Layer.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include "adi_platform.h"
#include "parameters.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"

/**
 * \brief Opens all neccessary files and device drivers for a specific device
 *
 * \param devHalCfg Pointer to device instance specific platform settings
 *
 * \retval ADI_HAL_OK Function completed successfully, no action required
 * \retval ADI_HAL_NULL_PTR The function has been called with a null pointer
 * \retval errors returned by other function calls.
 */
int32_t no_os_HwOpen(void *devHalCfg)
{
	int32_t ret;
	struct adi_hal *phal = (struct adi_hal *)devHalCfg;
	struct gpio_init_param gip_gpio_reset;
	struct xil_gpio_init_param gip_extra = {
#ifdef PLATFORM_MB
		.type = GPIO_PL,
#else
		.type = GPIO_PS,
#endif
		.device_id = GPIO_DEVICE_ID
	};

	/* Reset GPIO configuration */
	gip_gpio_reset.number = GPIO_RESET;
	gip_gpio_reset.extra = &gip_extra;
	ret = gpio_get(&phal->gpio_reset, &gip_gpio_reset);
	if (ret < 0)
		return ret;

	ret = gpio_set_value(phal->gpio_reset, GPIO_HIGH);
	if (ret < 0)
		return ret;

	return ADI_HAL_OK;
}

/*
 * Function pointer assignemt for default configuration
 */

/* Initialization interface to open, init, close drivers and pointers to resources */
int32_t (*adi_hal_HwOpen)(void *devHalCfg) = no_os_HwOpen;
// int32_t (*adi_hal_HwClose)(void *devHalCfg) = linux_HwClose;
// int32_t (*adi_hal_HwReset)(void *devHalCfg, uint8_t pinLevel) = linux_HwReset;

/* SPI Interface */
// int32_t (*adi_hal_SpiWrite)(void *devHalCfg, const uint8_t txData[], uint32_t numTxBytes) = linux_SpiWrite;
// int32_t (*adi_hal_SpiRead)(void *devHalCfg, const uint8_t txData[], uint8_t rxData[], uint32_t numRxBytes) = linux_SpiRead;

/* Logging interface */
// int32_t (*adi_hal_LogFileOpen)(void *devHalCfg, const char *filename) = linux_LogFileOpen;
// int32_t(*adi_hal_LogLevelSet)(void *devHalCfg, int32_t logLevel) = linux_LogLevelSet;
// int32_t(*adi_hal_LogLevelGet)(void *devHalCfg, int32_t *logLevel) = linux_LogLevelGet;
// int32_t(*adi_hal_LogWrite)(void *devHalCfg, int32_t logLevel, const char *comment, va_list args) = linux_LogWrite;
// int32_t(*adi_hal_LogFileClose)(void *devHalCfg) = linux_LogFileClose;

/* Timer interface */
// int32_t (*adi_hal_Wait_ms)(void *devHalCfg, uint32_t time_ms) = linux_TimerWait_ms;
// int32_t (*adi_hal_Wait_us)(void *devHalCfg, uint32_t time_us) = linux_TimerWait_us;

/* Mcs interface */
// int32_t(*adi_hal_Mcs_Pulse)(void *devHalCfg, uint8_t numberOfPulses) = linux_Mcs_Pulse;

/* ssi */
// int32_t(*adi_hal_ssi_Reset)(void *devHalCfg) = linux_ssi_Reset;

/* File IO abstraction */
// int32_t(*adi_hal_ArmImagePageGet)(void *devHalCfg, const char *ImagePath, uint32_t pageIndex, uint32_t pageSize, uint8_t *rdBuff) = linux_ImagePageGet;
// int32_t(*adi_hal_StreamImagePageGet)(void *devHalCfg, const char *ImagePath, uint32_t pageIndex, uint32_t pageSize, uint8_t *rdBuff) = linux_ImagePageGet;
// int32_t(*adi_hal_RxGainTableEntryGet)(void *devHalCfg, const char *rxGainTablePath, uint16_t lineCount, uint8_t *gainIndex, uint8_t *rxFeGain,
				//        uint8_t *tiaControl, uint8_t *adcControl, uint8_t *extControl, uint16_t *phaseOffset, int16_t *digGain) = linux_RxGainTableEntryGet;
// int32_t(*adi_hal_TxAttenTableEntryGet)(void *devHalCfg, const char *txAttenTablePath, uint16_t lineCount, uint16_t *attenIndex, uint8_t *txAttenHp,
				//        uint16_t *txAttenMult) = linux_TxAttenTableEntryGet;

/**
 * \brief Platform setup
 *
 * \param devHalInfo void pointer to be casted to the HAL config structure
 * \param platform Platform to be assigning the function pointers
 *
 * \return
 */
int32_t adi_hal_PlatformSetup(void *devHalInfo, adi_hal_Platforms_e platform)
{
	adi_hal_Err_e error = ADI_HAL_OK;

	adi_hal_HwOpen = no_os_HwOpen;
	/*
	adi_hal_HwClose = linux_HwClose;
	adi_hal_HwReset = linux_HwReset;

	adi_hal_SpiWrite = linux_SpiWrite;
	adi_hal_SpiRead = linux_SpiRead;

	adi_hal_LogFileOpen = linux_LogFileOpen;
	adi_hal_LogLevelSet = linux_LogLevelSet;
	adi_hal_LogLevelGet = linux_LogLevelGet;
	adi_hal_LogWrite = linux_LogWrite;
	adi_hal_LogFileClose = linux_LogFileClose;

	adi_hal_Wait_us = linux_TimerWait_us;
	adi_hal_Wait_ms = linux_TimerWait_ms;

	adi_hal_Mcs_Pulse = linux_Mcs_Pulse;

	adi_hal_ssi_Reset = linux_ssi_Reset;

	adi_hal_ArmImagePageGet = linux_ImagePageGet;
	adi_hal_StreamImagePageGet = linux_ImagePageGet;
	adi_hal_RxGainTableEntryGet = linux_RxGainTableEntryGet;
	adi_hal_TxAttenTableEntryGet = linux_TxAttenTableEntryGet;
	*/

	return error;
}
