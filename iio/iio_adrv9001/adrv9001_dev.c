/***************************************************************************//**
 *   @file   adrv9001_dev.c
 *   @brief  Implementation of adrv9001_dev.c.
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "adrv9001_dev.h"
#include "error.h"
#include "util.h"
#include "parameters.h"
#include "xil_cache.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief Transfer data from RAM to device.
 * @param iio_inst - Physical instance of a iio_adrv9001_dac device.
 * @param bytes_count - Number of bytes to transfer.
 * @param ch_mask - Opened channels mask.
 * @return Number of bytes transfered, or negative value in case of failure.
 */
ssize_t iio_adrv9001_transfer_mem_to_dev(void *iio_inst,
				     size_t bytes_count,
				     uint32_t ch_mask)
{
	struct iio_adrv9001_desc *adrv9001_device;
	adrv9001_device = (struct iio_adrv9001_desc *)iio_inst;
	if (!adrv9001_device)
		return -EINVAL;

	axi_dmac_transfer(adrv9001_device->dmac,
			  adrv9001_device->ddr_base_addr,
			  bytes_count);
	Xil_DCacheInvalidateRange(adrv9001_device->ddr_base_addr,
				  bytes_count);

	return bytes_count;
}

/**
 * @brief Transfer data from device into RAM.
 * @param iio_inst - Physical instance of a device.
 * @param bytes_count - Number of bytes to transfer.
 * @param ch_mask - Opened channels mask.
 * @return bytes_count or negative value in case of error.
 */
ssize_t iio_adrv9001_transfer_dev_to_mem(void *iio_inst,
				     size_t bytes_count,
				     uint32_t ch_mask)
{
	struct iio_adrv9001_desc *adrv9001_device;
	adrv9001_device = (struct iio_adrv9001_desc *)iio_inst;
	if (!adrv9001_device)
		return -EINVAL;

	axi_dmac_transfer(adrv9001_device->dmac,
			  adrv9001_device->ddr_base_addr,
			  bytes_count);
	Xil_DCacheInvalidateRange(adrv9001_device->ddr_base_addr,
				  bytes_count);
	return bytes_count;
}

/**
 * @brief Write chunk of data into RAM.
 * This function is probably called multiple times by libtinyiiod before a
 * "iio_transfer_mem_to_dev" call, since we can only write "bytes_count" bytes
 * at a time.
 * @param iio_inst - Physical instance of a iio_adrv9001_dac device.
 * @param buf - Values to write.
 * @param offset - Offset in memory after the nth chunk of data.
 * @param bytes_count - Number of bytes to write.
 * @param ch_mask - Opened channels mask.
 * @return bytes_count or negative value in case of error.
 */
ssize_t iio_adrv9001_write_dev(void *iio_inst, char *buf,
			   size_t offset,  size_t bytes_count, uint32_t ch_mask)
{
	struct iio_adrv9001_desc *adrv9001_device;
	uint32_t index, addr;
	uint16_t *buf16;

	if (!iio_inst || !buf)
		return -EINVAL;

	buf16 = (uint16_t *)buf;
	adrv9001_device = (struct iio_adrv9001_desc *)iio_inst;
	addr = adrv9001_device->ddr_base_addr + offset;
	for(index = 0; index < bytes_count; index += 2) {
		uint32_t *local_addr = (uint32_t *)(addr +
						    (index * 2) % adrv9001_device->ddr_base_size);
		*local_addr = (buf16[index + 1] << 16) | buf16[index];
	}


	return bytes_count;
}

/**
 * @brief Read chunk of data from RAM to pbuf.
 * Call "iio_adrv9001_transfer_dev_to_mem" first.
 * This function is probably called multiple times by libtinyiiod after a
 * "iio_adrv9001_transfer_dev_to_mem" call, since we can only read "bytes_count"
 * bytes at a time.
 * @param iio_inst - Physical instance of a device.
 * @param pbuf - Buffer where value is stored.
 * @param offset - Offset to the remaining data after reading n chunks.
 * @param bytes_count - Number of bytes to read.
 * @param ch_mask - Opened channels mask.
 * @return bytes_count or negative value in case of error.
 */
ssize_t iio_adrv9001_read_dev(void *iio_inst, char *pbuf, size_t offset,
			  size_t bytes_count, uint32_t ch_mask)
{
	struct iio_adrv9001_desc *adrv9001_device;
	uint32_t i, j = 0, current_ch = 0;
	uint16_t *pbuf16;
	size_t samples;

	if (!iio_inst || !pbuf)
		return -EINVAL;

	adrv9001_device = (struct iio_adrv9001_desc*)iio_inst;
	pbuf16 = (uint16_t*)pbuf;
	samples = (bytes_count * ADRV9001_NUM_CHANNELS) / hweight8(
			  ch_mask);
	samples /= 2;
	offset = (offset * ADRV9001_NUM_CHANNELS) / hweight8(ch_mask);

	for (i = 0; i < samples; i++) {

		if (ch_mask & BIT(current_ch)) {
			pbuf16[j] = *(uint16_t*)(adrv9001_device->ddr_base_addr +
						 (offset + i * 2) % adrv9001_device->ddr_base_size);

			j++;
		}

		if (current_ch + 1 < ADRV9001_NUM_CHANNELS)
			current_ch++;
		else
			current_ch = 0;
	}

	return bytes_count;
}

/**
 * @brief iio adrv9001 init function, registers a adrv9001 .
 * @param desc - Descriptor.
 * @param init - Configuration structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t iio_adrv9001_dev_init(struct iio_adrv9001_desc **desc,
			  struct iio_adrv9001_init_param *init)
{
	struct iio_adrv9001_desc *ldesc;
	int ret;

	ldesc = (struct iio_adrv9001_desc*)calloc(1, sizeof(*ldesc));
	if (!ldesc)
		return -ENOMEM;

	ldesc->ddr_base_addr = init->ddr_base_addr;
	ldesc->ddr_base_size = init->ddr_base_size;

	ret = axi_dmac_init(&ldesc->dmac, &init->dmac_init);
	if (ret)
		goto error;

	*desc = ldesc;

	return ret;
error:
	free(ldesc);
	return ret;
}

/**
 * @brief Release resources.
 * @param desc - Descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t iio_adrv9001_dev_remove(struct iio_adrv9001_desc *desc)
{
	if (!desc)
		return -EINVAL;

	free(desc);

	return SUCCESS;
}
