/**
* Copyright 2015 - 2018 Analog Devices Inc.
* Released under the ADRV9001 API license, for more information.
* see the "LICENSE.txt" file in this zip file.
*/

#include "adi_platform.h"

#ifdef _ADI_SW_TEST
#include "swtest/swtest_adihal_interface.h"
#include "swtest/swtest_fpga_interface.h"
#include "swtest/swtest_fpga.h"
#endif

#ifdef _ADI_STS_PLATFORM 
#include "swtest/swtest_adihal_interface.h"
#include "swtest/swtest_fpga_interface.h"
#include "swtest/swtest_fpga.h"
#include "sts_hal/sts_init.h"
#include "sts_hal/sts_spi.h"
#include "sts_hal/sts_timer.h"
#include "sts_hal/sts_logging.h"
#endif

#include "zc706sd20/zc706sd20_init.h"
#include "zc706sd20/zc706sd20_spi.h"
#include "zc706sd20/zc706sd20_i2c.h"
#include "zc706sd20/zc706sd20_logging.h"
#include "zc706sd20/zc706sd20_timer.h"
#include "zc706sd20/zc706sd20_bbic_control.h"
#include "zc706sd20/zc706sd20_mcs.h"
#include "zc706sd20/zc706sd20_ssi.h"



/*
 * Function pointer assignemt for default configuration
 */

/* Initialization interface to open, init, close drivers and pointers to resources */
int32_t (*adi_hal_HwOpen)(void *devHalCfg) = NULL;
int32_t(*adi_hal_HwClose)(void *devHalCfg) = NULL;
int32_t(*adi_hal_HwReset)(void *devHalCfg, uint8_t pinLevel) = NULL;
void* (*adi_hal_DevHalCfgCreate)(uint32_t interfaceMask, uint8_t spiChipSelect, const char *logFilename) = NULL;
int32_t(*adi_hal_DevHalCfgFree)(void *devHalCfg) = NULL;

/* SPI Interface */
int32_t(*adi_hal_SpiWrite)(void *devHalCfg, const uint8_t txData[], uint32_t numTxBytes) = NULL;
int32_t(*adi_hal_SpiRead)(void *devHalCfg, const uint8_t txData[], uint8_t rxData[], uint32_t numRxBytes) = NULL;

/* I2C Interface */
int32_t(*adi_hal_I2C_Write)(void *devHalCfg, uint8_t slaveAddress, const uint8_t wrData[], uint32_t numWrBytes) = NULL;
int32_t(*adi_hal_I2C_Read)(void *devHalCfg, uint8_t slaveAddress, const uint8_t txData[], uint32_t numTxBytes, uint8_t rxData[], uint32_t numRxBytes) = NULL;

/* Logging interface */
int32_t(*adi_hal_LogFileOpen)(void *devHalCfg, const char *filename) = NULL;
int32_t(*adi_hal_LogLevelSet)(void *devHalCfg, int32_t logLevel) = NULL;
int32_t(*adi_hal_LogLevelGet)(void *devHalCfg, int32_t *logLevel) = NULL;
int32_t(*adi_hal_LogWrite)(void *devHalCfg, int32_t logLevel, const char *comment, va_list args) = NULL; 
int32_t(*adi_hal_LogFileClose)(void *devHalCfg) = NULL;

/* Timer interface */
int32_t(*adi_hal_Wait_ms)(void *devHalCfg, uint32_t time_ms) = NULL;
int32_t(*adi_hal_Wait_us)(void *devHalCfg, uint32_t time_us) = NULL;

/* File IO abstraction */
int32_t(*adi_hal_ArmImagePageGet)(void *devHalCfg, const char *armImagePath, uint32_t pageIndex, uint32_t pageSize, uint8_t *rdBuff) = NULL;
int32_t(*adi_hal_StreamImagePageGet)(void *devHalCfg, const char *streamImagePath, uint32_t pageIndex, uint32_t pageSize, uint8_t *rdBuff) = NULL;
int32_t(*adi_hal_RxGainTableEntryGet)(void *devHalCfg, const char *rxGainTablePath, uint16_t lineCount, uint8_t *gainIndex, uint8_t *rxFeGain, uint8_t *tiaControl, uint8_t *adcControl, uint8_t *extControl, uint16_t *phaseOffset, int16_t *digGain) = NULL;
int32_t(*adi_hal_TxAttenTableEntryGet)(void *devHalCfg, const char *txAttenTablePath, uint16_t lineCount, uint16_t *attenIndex, uint8_t *txAttenHp, uint16_t *txAttenMult) = NULL;

/* only required to support the FPGA / BBIC control interface */
int32_t(*adi_hal_BbicRegisterRead)(void *devHalCfg, uint32_t addr, uint32_t *data) = NULL;
int32_t(*adi_hal_BbicRegisterWrite)(void *devHalCfg, uint32_t addr, uint32_t data) = NULL;
int32_t(*adi_hal_BbicRegistersRead)(void *devHalCfg, uint32_t addr, uint32_t data[], uint32_t numDataWords) = NULL;
int32_t(*adi_hal_BbicRegistersWrite)(void *devHalCfg, uint32_t addr, uint32_t data[], uint32_t numDataWords) = NULL;

int32_t(*adi_hal_Mcs_Pulse)(void *devHalCfg, uint8_t numberOfPulses) = NULL;

int32_t(*adi_hal_ssi_Reset)(void *devHalCfg) = NULL;

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
    switch (platform)
    {
    case ADI_HW_PLATFORM:
        adi_hal_HwOpen = zc706sd20_HwOpen;
        adi_hal_HwClose = zc706sd20_HwClose;
        adi_hal_HwReset = zc706sd20_HwReset;
        adi_hal_DevHalCfgCreate = zc706sd20_DevHalCfgCreate;
        adi_hal_DevHalCfgFree = zc706sd20_DevHalCfgFree;
        
        adi_hal_SpiWrite = zc706sd20_SpiWrite;
        adi_hal_SpiRead = zc706sd20_SpiRead;
        
        adi_hal_I2C_Write = zc706sd20_I2cWrite;
        adi_hal_I2C_Read = zc706sd20_I2cRead;

        adi_hal_LogFileOpen = zc706sd20_LogFileOpen;
        adi_hal_LogLevelSet = zc706sd20_LogLevelSet;
        adi_hal_LogLevelGet = zc706sd20_LogLevelGet;
        adi_hal_LogWrite = zc706sd20_LogWrite;
        adi_hal_LogFileClose = zc706sd20_LogFileClose;
        
        
        adi_hal_Wait_us = zc706sd20_TimerWait_us;
        adi_hal_Wait_ms = zc706sd20_TimerWait_ms;

        /* only required to support the ADI FPGA*/
        adi_hal_BbicRegisterRead   = zc706sd20_BbicRegisterRead;
        adi_hal_BbicRegisterWrite  = zc706sd20_BbicRegisterWrite;
        adi_hal_BbicRegistersRead  = zc706sd20_BbicRegistersRead;
        adi_hal_BbicRegistersWrite = zc706sd20_BbicRegistersWrite;
        
        adi_hal_Mcs_Pulse = zc706sd20_Mcs_Pulse;
        
        adi_hal_ssi_Reset = zc706sd20_ssi_Reset;
        
        break;

    case ADI_SW_PLATFORM:
#ifdef _ADI_SW_TEST
        adi_hal_HwOpen = swtest_adihal_HwOpen;
        adi_hal_HwClose = swtest_adihal_HwClose;
        adi_hal_HwReset = swtest_adihal_HwReset;
        adi_hal_DevHalCfgCreate = swtest_adihal_DevHalCfgCreate;
        adi_hal_DevHalCfgFree = swtest_adihal_DevHalCfgFree;
        
        adi_hal_SpiWrite = swtest_adihal_SpiWrite;
        adi_hal_SpiRead = swtest_adihal_SpiRead;

        adi_hal_LogFileOpen = swtest_adihal_LogFileOpen;
        adi_hal_LogLevelSet = swtest_adihal_LogLevelSet;
        adi_hal_LogLevelGet = swtest_LogLevelGet; /* TODO: implement this function */
        adi_hal_LogWrite = zc706sd20_LogWrite;
        adi_hal_LogFileClose = zc706sd20_LogFileClose;
        
        adi_hal_Wait_us = swtest_adihal_Wait_us;
        adi_hal_Wait_ms = swtest_adihal_Wait_ms;

        /* TODO: implement these function */
        adi_hal_BbicRegisterRead   = swtest_BbicRegisterRead;
        adi_hal_BbicRegisterWrite  = swtest_BbicRegisterWrite;
        adi_hal_BbicRegistersRead  = swtest_BbicRegistersRead;
        adi_hal_BbicRegistersWrite = swtest_BbicRegistersWrite;
        
#else
        error = ADI_HAL_LIBRARY_NOT_AVAILABLE;
#endif
        break;

    case ADI_STS_PLATFORM:
#ifdef _ADI_STS_PLATFORM
        adi_hal_BbicRegisterRead = swtest_BbicRegisterRead;
        adi_hal_BbicRegisterWrite = swtest_BbicRegisterWrite;
        adi_hal_BbicRegistersRead = swtest_BbicRegistersRead;
        adi_hal_BbicRegistersWrite = swtest_BbicRegistersWrite;
 
        adi_hal_HwOpen = sts_HwOpen;
        adi_hal_HwClose = sts_HwClose;
        adi_hal_HwReset = sts_HwReset;
        adi_hal_SpiWrite = sts_SpiWrite;
        adi_hal_SpiRead = sts_SpiRead;

        adi_hal_LogFileOpen = sts_LogFileOpen;
        adi_hal_LogLevelSet = sts_LogLevelSet;
        adi_hal_LogLevelGet = sts_LogLevelGet;
        adi_hal_LogWrite = sts_LogWrite;
        adi_hal_LogFileClose = sts_LogFileClose;

        adi_hal_Wait_us = sts_Wait_us;
        adi_hal_DevHalCfgCreate = sts_DevHalCfgCreate;
        adi_hal_DevHalCfgFree = sts_DevHalCfgFree;
#else
        error = ADI_HAL_LIBRARY_NOT_AVAILABLE;
#endif
        break;

    default:
        error = ADI_HAL_GEN_SW;
        break;
    }

    return error;
}

