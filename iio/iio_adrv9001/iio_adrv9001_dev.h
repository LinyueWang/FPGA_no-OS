/***************************************************************************//**
 *   @file   iio_adrv9001_dev.h
 *   @brief  iio description of adrv9001_dev
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

#ifndef IIO_ADRV9001_DEV
#define IIO_ADRV9001_DEV

#include <stdlib.h>
#include "adrv9001_dev.h"
#include "xil_cache.h"

static struct iio_channel iio_adrv9001_channel_voltage0_i_in = {
	.name = "voltage0_i",
	.scan_index = 0,
	.scan_type = {
		.sign = 's',
		.realbits = 16,
		.storagebits = 16,
		.shift = 0,
		.is_big_endian = false
	},
	.ch_out = false,
};

static struct iio_channel iio_adrv9001_channel_voltage0_q_in = {
	.name = "voltage0_q",
	.scan_index = 0,
	.scan_type = {
		.sign = 's',
		.realbits = 16,
		.storagebits = 16,
		.shift = 0,
		.is_big_endian = false
	},
	.ch_out = false,
};

static struct iio_channel iio_adrv9001_channel_voltage0_i_out = {
	.name = "voltage0_i",
	.scan_index = 0,
	.scan_type = {
		.sign = 's',
		.realbits = 16,
		.storagebits = 16,
		.shift = 0,
		.is_big_endian = false
	},
	.ch_out = true,
};

static struct iio_channel iio_adrv9001_channel_voltage0_q_out = {
	.name = "voltage0_q",
	.scan_index = 0,
	.scan_type = {
		.sign = 's',
		.realbits = 16,
		.storagebits = 16,
		.shift = 0,
		.is_big_endian = false
	},
	.ch_out = true,
};

static struct iio_channel *iio_adrv9001_channels_in[] = {
	&iio_adrv9001_channel_voltage0_i_in,
	&iio_adrv9001_channel_voltage0_q_in,
	NULL,
};

static struct iio_channel *iio_adrv9001_channels_out[] = {
	&iio_adrv9001_channel_voltage0_i_out,
	&iio_adrv9001_channel_voltage0_q_out,
	NULL,
};

static struct iio_device iio_adrv9001_dev_in_descriptor = {
	.num_ch = ADRV9001_NUM_CHANNELS,
	.channels = iio_adrv9001_channels_in,
	.debug_attributes = NULL,
	.buffer_attributes = NULL,
	.transfer_dev_to_mem = iio_adrv9001_transfer_dev_to_mem,
	.transfer_mem_to_dev = iio_adrv9001_transfer_mem_to_dev,
	.read_data = iio_adrv9001_read_dev,
	.write_data = iio_adrv9001_write_dev
};

static struct iio_device iio_adrv9001_dev_out_descriptor = {
	.num_ch = ADRV9001_NUM_CHANNELS,
	.channels = iio_adrv9001_channels_out,
	.debug_attributes = NULL,
	.buffer_attributes = NULL,
	.transfer_dev_to_mem = iio_adrv9001_transfer_dev_to_mem,
	.transfer_mem_to_dev = iio_adrv9001_transfer_mem_to_dev,
	.read_data = iio_adrv9001_read_dev,
	.write_data = iio_adrv9001_write_dev
};

#endif /* IIO_ADRV9001_DEV */
