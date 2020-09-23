/***************************************************************************//**
*   @file   adrv9002_dev.h
*   @brief  Header file of iio_adrv9001
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

#ifndef IIO_ADRV9001_H_
#define IIO_ADRV9001_H_

#include <stdio.h>
#include "iio_types.h"
#include "axi_dmac.h"

#define ADRV9001_NUM_CHANNELS	2

/**
 * @struct iio_adrv9001_desc
 * @brief Desciptor.
 */
struct iio_adrv9001_desc {
	/** Address used by for reading/writing data to device */
	uint32_t ddr_base_addr;
	/** Size of memory to read/write data */
	uint32_t ddr_base_size;
	/** DMA Controller descriptor */
	struct axi_dmac *dmac;
};

/**
 * @struct iio_adrv9001_init_param
 * @brief iio adrv9001 configuration.
 */
struct iio_adrv9001_init_param {
	/** Address used by for reading/writing data to device */
	uint32_t ddr_base_addr;
	/** Size of memory to read/write data */
	uint32_t ddr_base_size;
	/** DMA Controller parameters */
	struct axi_dmac_init dmac_init;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

ssize_t iio_adrv9001_transfer_mem_to_dev(void *iio_inst,
				     size_t bytes_count,
				     uint32_t ch_mask);
ssize_t iio_adrv9001_transfer_dev_to_mem(void *iio_inst,
				     size_t bytes_count,
				     uint32_t ch_mask);
ssize_t iio_adrv9001_write_dev(void *iio_inst, char *buf,
			   size_t offset,  size_t bytes_count, uint32_t ch_mask);
ssize_t iio_adrv9001_read_dev(void *iio_inst, char *pbuf, size_t offset,
			  size_t bytes_count, uint32_t ch_mask);

/* Init function. */
int32_t iio_adrv9001_dev_init(struct iio_adrv9001_desc **desc,
			  struct iio_adrv9001_init_param *param);
/* Free the resources allocated by iio_adrv9001_init(). */
int32_t iio_adrv9001_dev_remove(struct iio_adrv9001_desc *desc);

#endif /* IIO_ADRV9001_H_ */
