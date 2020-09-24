#include "error.h"
#include "uart.h"
#include "uart_extra.h"
#include "iio_app.h"
#include "parameters.h"
#include "app_iio.h"
#include "irq.h"
#include "irq_extra.h"

static struct uart_desc *uart_desc;

/**
 * iio_server_write() - Write data to UART.
 * @buf - Data to be written.
 * @len - Number of bytes to be written.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_server_write(const char *buf, size_t len)
{
	return uart_write(uart_desc, (const uint8_t *)buf, len);
}

/**
 * iio_server_read() - Read data from UART.
 * @buf - Storing data location.
 * @len - Number of bytes to be read.
 * @Return: SUCCESS in case of success, FAILURE otherwise.
 */
static ssize_t iio_server_read(char *buf, size_t len)
{
	return uart_read(uart_desc, (uint8_t *)buf, len);
}

/**
 * @brief Application IIO setup.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t iio_server_init(struct iio_axi_adc_init_param *adc_init,
			struct iio_axi_dac_init_param *dac_init)
{
	struct xil_uart_init_param xil_uart_init_par = {
		.type = UART_PS,
		.irq_id = UART_IRQ_ID,
	};
	struct uart_init_param uart_init_par = {
		.baud_rate = 115200,
		.device_id = UART_DEVICE_ID,
		.extra = &xil_uart_init_par,
	};
	struct iio_server_ops uart_iio_server_ops = {
		.read = iio_server_read,
		.write = iio_server_write,
	};
	struct iio_app_init_param iio_app_init_par = {
		.iio_server_ops = &uart_iio_server_ops,
	};
	struct iio_app_desc *iio_app_desc;
	struct iio_axi_adc_desc *iio_axi_adc_desc;
	struct iio_axi_dac_desc *iio_axi_dac_desc;
	int32_t status;

	status = uart_init(&uart_desc, &uart_init_par);
	if (status < 0)
		return status;

	status = iio_app_init(&iio_app_desc, &iio_app_init_par);
	if (status < 0)
		return status;

	status = iio_axi_adc_init(&iio_axi_adc_desc, adc_init);
	if (status < 0)
		return status;

	status = iio_axi_dac_init(&iio_axi_dac_desc, dac_init);
	if (status < 0)
		return status;

	return iio_app(iio_app_desc);
}
