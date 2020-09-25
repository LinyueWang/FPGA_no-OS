#ifndef APP_IIO_H_
#define APP_IIO_H_

#include <stdint.h>
#include "iio_axi_adc.h"
#include "iio_axi_dac.h"

/* @brief Application IIO setup. */
int32_t iio_server_init(struct iio_axi_adc_init_param *adc1_init,
			struct iio_axi_adc_init_param *adc2_init,
			struct iio_axi_dac_init_param *dac1_init,
			struct iio_axi_dac_init_param *dac2_init);

#endif