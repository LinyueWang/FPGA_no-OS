/***************************************************************************//**
 *   @file   spi.h
 *   @brief  Header file of SPI Interface
 *   @author DBogdan (dragos.bogdan@analog.com)
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

#ifndef SPI_H_
#define SPI_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "util.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define	SPI_CPHA	0x01
#define	SPI_CPOL	0x02
#define	SPI_CS_LOW	1
#define	SPI_CS_HIGH	2

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/**
 * @enum spi_mode
 * @brief SPI configuration for clock phase and polarity.
 */
typedef enum spi_mode {
	/** Data on rising, shift out on falling */
	SPI_MODE_0 = (0 | 0),
	/** Data on falling, shift out on rising */
	SPI_MODE_1 = (0 | SPI_CPHA),
	/** Data on falling, shift out on rising */
	SPI_MODE_2 = (SPI_CPOL | 0),
	/** Data on rising, shift out on falling */
	SPI_MODE_3 = (SPI_CPOL | SPI_CPHA)
} spi_mode;

/**
 * @enum spi_delay_unit
 * @brief Time unit used by the spi_delay structure.
 */
enum spi_delay_unit {
	SPI_DELAY_UNIT_NS = 1000000,
	SPI_DELAY_UNIT_US = 1000,
	SPI_DELAY_UNIT_MS = 1
};

/**
 * @struct spi_init_param
 * @brief Structure holding the parameters for SPI initialization
 */
typedef struct spi_init_param {
	/** maximum transfer speed */
	uint32_t	max_speed_hz;
	/** SPI chip select */
	uint8_t		chip_select;
	/** SPI mode */
	enum spi_mode	mode;
	/**  SPI extra parameters (device specific) */
	void		*extra;
} spi_init_param;

/**
 * @struct spi_desc
 * @brief Structure holding SPI descriptor.
 */
typedef struct spi_desc {
	/** maximum transfer speed */
	uint32_t	max_speed_hz;
	/** SPI chip select */
	uint8_t		chip_select;
	/** SPI mode */
	enum spi_mode	mode;
	/**  SPI extra parameters (device specific) */
	void		*extra;
} spi_desc;

/**
 * @struct spi_delay
 * @brief Delay structure containing the amount of time
 * 	the SPI interface has to hang among with it's
 * 	measurement unit.
 */
struct spi_delay {
	uint32_t		value;
	enum spi_delay_unit	unit;
};

/**
 * @struct spi_sequence
 * @brief An spi sequence represents a whole (or a part of an)
 * 	SPI transaction.
 */
struct spi_sequence {
	/** Amount of time to hang a spi transfer */
	struct spi_delay	delay;
	/** Address of the MOSI data buffer */
	void 			*tx_buff;
	/** Address of the MISO data buffer */
	void			*rx_buff;
	/** The width of one transfer WORD represented in bits */
	uint8_t			word_width_bits;
	/** Number of WORDS to transfer */
	uint32_t		length;
	/** Toggle or not the CHIP SELECT */
	uint32_t		cs;
	/** Pointer to the next sequence in case of a transfer queue */
	struct spi_sequence	*next_sequence;
};

/**
 * @struct spi_transfer
 * @brief An spi transfer contains a queue of spi sequences, ready
 * 	to be sent over the SPI interface.
 */
struct spi_transfer {
	/** List of spi sequences */
	struct spi_sequence	*sequence_list;
	/** DMAC used to memory map the MOSI data */
	void			*tx_dma_baseaddr;
	/** DMAC used to memory map the MISO data */
	void			*rx_dma_baseaddr;
	/** Specify if the transfer is memory mapped */
	bool			is_memory_mapped;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Initialize the SPI communication peripheral. */
int32_t spi_init(struct spi_desc **desc,
		 const struct spi_init_param *param);

/* Free the resources allocated by spi_init(). */
int32_t spi_remove(struct spi_desc *desc);

/* Write and read data to/from SPI. */
int32_t spi_write_and_read(struct spi_desc *desc,
			   uint8_t *data,
			   uint16_t bytes_number);

/* Initialize the spi message and add a spi transfer to it */
int32_t spi_prepare_transfer(struct spi_transfer **xfer,
			     struct spi_sequence *seq);

/* Add a spi transfer to the spi message's transfer queue */
int32_t spi_add_sequence(struct spi_transfer *xfer,
			 struct spi_sequence *seq);

/* Transfer the spi message using the specified device descriptor */
int32_t spi_start_transfer(struct spi_desc *desc,
			   struct spi_transfer *xfer);
#endif // SPI_H_
