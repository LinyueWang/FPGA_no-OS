#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "xil_cache.h"

#include "error.h"
#include "util.h"
#include "spi.h"

#include "adrv9002.h"
#include "adi_adrv9001.h"
#include "adi_adrv9001_arm.h"
#include "adi_adrv9001_arm_types.h"
#include "adi_adrv9001_bbdc.h"
#include "adi_adrv9001_cals.h"
#include "adi_adrv9001_cals_types.h"
#include "adi_common_types.h"
#include "adi_adrv9001_gpio.h"
#include "adi_adrv9001_gpio_types.h"
#include "adi_adrv9001_profile_types.h"
#include "adi_adrv9001_radio.h"
#include "adi_adrv9001_radio_types.h"
#include "adi_adrv9001_rx_gaincontrol.h"
#include "adi_adrv9001_rx_gaincontrol_types.h"
#include "adi_adrv9001_rx.h"
#include "adi_adrv9001_rx_types.h"
#include "adi_adrv9001_rxSettings_types.h"
#include "adi_adrv9001_types.h"
#include "adi_adrv9001_tx.h"
#include "adi_adrv9001_tx_types.h"
#include "adi_adrv9001_txSettings_types.h"
#include "adi_adrv9001_utilities.h"
#include "adi_adrv9001_utilities_types.h"
#include "adi_adrv9001_version.h"
#include "adi_common_error_types.h"
#include "adi_platform_types.h"

#include "axi_adc_core.h"
#include "axi_dac_core.h"
#include "axi_dmac.h"

#include "parameters.h"

#ifdef IIO_SUPPORT
#include "app_iio.h"
#include "xil_cache.h"
#endif

/* gpio0 starts at 1 in the API enum */
#define ADRV9002_DGPIO_MIN	(ADI_ADRV9001_GPIO_DIGITAL_00 - 1)
#define ADRV9002_DGPIO_MAX	(ADI_ADRV9001_GPIO_DIGITAL_15 - 1)

#define ALL_RX_CHANNEL_MASK	(ADI_ADRV9001_RX1 | ADI_ADRV9001_RX2 | \
				 ADI_ADRV9001_ORX1 | ADI_ADRV9001_ORX2)

#define ADRV9002_RX_EN(nr)	BIT(((nr) * 2) & 0x3)
#define ADRV9002_TX_EN(nr)	BIT(((nr) * 2 + 1) & 0x3)

#define ADRV9002_RX_MAX_GAIN_mdB	36000
#define ADRV9002_RX_GAIN_STEP_mDB	500
#define ADRV9002_RX_MIN_GAIN_IDX	183
#define ADRV9002_RX_MAX_GAIN_IDX	255

/* IRQ Masks */
#define ADRV9002_GP_MASK_RX_DP_RECEIVE_ERROR		0x08000000
#define ADRV9002_GP_MASK_TX_DP_TRANSMIT_ERROR		0x04000000
#define ADRV9002_GP_MASK_RX_DP_READ_REQUEST_FROM_BBIC	0x02000000
#define ADRV9002_GP_MASK_TX_DP_WRITE_REQUEST_TO_BBIC	0x01000000
#define ADRV9002_GP_MASK_STREAM_PROCESSOR_3_ERROR	0x00100000
#define ADRV9002_GP_MASK_STREAM_PROCESSOR_2_ERROR	0x00080000
#define ADRV9002_GP_MASK_STREAM_PROCESSOR_1_ERROR	0x00040000
#define ADRV9002_GP_MASK_STREAM_PROCESSOR_0_ERROR	0x00020000
#define ADRV9002_GP_MASK_MAIN_STREAM_PROCESSOR_ERROR	0x00010000
#define ADRV9002_GP_MASK_LSSI_RX2_CLK_MCS		0x00008000
#define ADRV9002_GP_MASK_LSSI_RX1_CLK_MCS		0x00004000
#define ADRV9002_GP_MASK_CLK_1105_MCS_SECOND		0x00002000
#define ADRV9002_GP_MASK_CLK_1105_MCS			0x00001000
#define ADRV9002_GP_MASK_CLK_PLL_LOCK			0x00000800
#define ADRV9002_GP_MASK_AUX_PLL_LOCK			0x00000400
#define ADRV9002_GP_MASK_RF2_SYNTH_LOCK			0x00000200
#define ADRV9002_GP_MASK_RF_SYNTH_LOCK			0x00000100
#define ADRV9002_GP_MASK_CLK_PLL_LOW_POWER_LOCK		0x00000080
#define ADRV9002_GP_MASK_TX2_PA_PROTECTION_ERROR	0x00000040
#define ADRV9002_GP_MASK_TX1_PA_PROTECTION_ERROR	0x00000020
#define ADRV9002_GP_MASK_CORE_ARM_MONITOR_ERROR		0x00000010
#define ADRV9002_GP_MASK_CORE_ARM_CALIBRATION_ERROR	0x00000008
#define ADRV9002_GP_MASK_CORE_ARM_SYSTEM_ERROR		0x00000004
#define ADRV9002_GP_MASK_CORE_FORCE_GP_INTERRUPT	0x00000002
#define ADRV9002_GP_MASK_CORE_ARM_ERROR			0x00000001

#define ADRV9002_IRQ_MASK					\
	(ADRV9002_GP_MASK_CORE_ARM_ERROR |			\
	 ADRV9002_GP_MASK_CORE_FORCE_GP_INTERRUPT |		\
	 ADRV9002_GP_MASK_CORE_ARM_SYSTEM_ERROR |		\
	 ADRV9002_GP_MASK_CORE_ARM_CALIBRATION_ERROR |		\
	 ADRV9002_GP_MASK_CORE_ARM_MONITOR_ERROR |		\
	 ADRV9002_GP_MASK_TX1_PA_PROTECTION_ERROR |		\
	 ADRV9002_GP_MASK_TX2_PA_PROTECTION_ERROR |		\
	 ADRV9002_GP_MASK_CLK_PLL_LOW_POWER_LOCK |		\
	 ADRV9002_GP_MASK_RF_SYNTH_LOCK |			\
	 ADRV9002_GP_MASK_RF2_SYNTH_LOCK |			\
	 ADRV9002_GP_MASK_AUX_PLL_LOCK |			\
	 ADRV9002_GP_MASK_CLK_PLL_LOCK |			\
	 ADRV9002_GP_MASK_MAIN_STREAM_PROCESSOR_ERROR |		\
	 ADRV9002_GP_MASK_STREAM_PROCESSOR_0_ERROR |		\
	 ADRV9002_GP_MASK_STREAM_PROCESSOR_1_ERROR |		\
	 ADRV9002_GP_MASK_STREAM_PROCESSOR_2_ERROR |		\
	 ADRV9002_GP_MASK_STREAM_PROCESSOR_3_ERROR |		\
	 ADRV9002_GP_MASK_TX_DP_WRITE_REQUEST_TO_BBIC |		\
	 ADRV9002_GP_MASK_RX_DP_READ_REQUEST_FROM_BBIC |	\
	 ADRV9002_GP_MASK_TX_DP_TRANSMIT_ERROR |		\
	 ADRV9002_GP_MASK_RX_DP_RECEIVE_ERROR)

#define ADRV9001_BF_EQUAL(mask, value) ((value) == ((value) & (mask)))

/* init data stuff */
extern struct adi_adrv9001_SpiSettings *adrv9002_spi_settings_get(void);
extern struct adi_adrv9001_Init *adrv9002_init_get(void);
extern struct adi_adrv9001_RadioCtrlInit *adrv9002_radio_ctrl_init_get(void);
extern struct adi_adrv9001_PlatformFiles *adrv9002_platform_files_get(void);
extern void adrv9002_cmos_default_set(void);
extern adi_adrv9001_Init_t adrv9002_init_lvds;
extern adi_adrv9001_Init_t adrv9002_init_cmos;

#ifdef DAC_DMA_EXAMPLE
extern const uint32_t sine_lut_iq[1024];
#endif

int __adrv9002_dev_err(const struct adrv9002_rf_phy *phy,
			      const char *function, const int line)
{
	int ret;

	printf("%s, %d: failed with \"%s\" (%d)\n", function, line,
		phy->adrv9001->common.error.errormessage ?
		phy->adrv9001->common.error.errormessage : "",
		phy->adrv9001->common.error.errCode);

	switch (phy->adrv9001->common.error.errCode) {
	case ADI_COMMON_ERR_INV_PARAM:
	case ADI_COMMON_ERR_NULL_PARAM:
		ret = -EINVAL;
	case ADI_COMMON_ERR_API_FAIL:
		ret = -EFAULT;
	case ADI_COMMON_ERR_SPI_FAIL:
		ret = -EIO;
	case ADI_COMMON_ERR_MEM_ALLOC_FAIL:
		ret = -ENOMEM;
	default:
		ret = -EFAULT;
	}

	adi_common_ErrorClear(&phy->adrv9001->common);

	return ret;
}

static int adrv9001_rx_path_config(struct adrv9002_rf_phy *phy,
				   const adi_adrv9001_ChannelState_e state)
{
	struct adi_adrv9001_Device *adrv9001_dev = phy->adrv9001;
	int ret;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(phy->rx_channels); i++) {
		struct adrv9002_rx_chan *rx = &phy->rx_channels[i];

		/* For each rx channel enabled */
		if (!rx->channel.enabled)
			continue;

		if (!rx->pin_cfg)
			goto agc_cfg;

		ret = adi_adrv9001_Rx_GainControl_PinMode_Configure(phy->adrv9001,
								    rx->channel.number,
								    rx->pin_cfg);
		if (ret)
			return adrv9002_dev_err(phy);

agc_cfg:
		if (!rx->agc)
			goto rf_enable;

		ret = adi_adrv9001_Rx_GainControl_Configure(phy->adrv9001,
							    rx->channel.number,
							    rx->agc);
		if (ret)
			return adrv9002_dev_err(phy);

rf_enable:
		ret = adi_adrv9001_Radio_Channel_ToState(adrv9001_dev, ADI_RX,
							 rx->channel.number, state);
		if (ret)
			return adrv9002_dev_err(phy);
	}

	return 0;
}

static int adrv9002_tx_set_dac_full_scale(struct adrv9002_rf_phy *phy)
{
	int ret = 0;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(phy->tx_channels); i++) {
		struct adrv9002_tx_chan *tx = &phy->tx_channels[i];

		if (!tx->channel.enabled || !tx->dac_boost_en)
			continue;

		ret = adi_adrv9001_Tx_OutputPowerBoost_Set(phy->adrv9001,
							   tx->channel.number,
							   true);
		if (ret)
			return adrv9002_dev_err(phy);
	}

	return ret;
}

static int adrv9002_tx_path_config(struct adrv9002_rf_phy *phy,
				   const adi_adrv9001_ChannelState_e state)
{
	int ret;
	unsigned int i;
	struct adi_adrv9001_TxProfile *profi = phy->curr_profile->tx.txProfile;

	for (i = 0; i < ARRAY_SIZE(phy->tx_channels); i++) {
		struct adrv9002_tx_chan *tx = &phy->tx_channels[i];
		struct adi_adrv9001_Info *info = &phy->adrv9001->devStateInfo;
		/* For each tx channel enabled */
		if (!tx->channel.enabled)
			continue;
		/*
		 * Should this be done by the API? This seems to be needed for
		 * the NCO tone generation. We need to clarify if this will be
		 * done by the API in future releases.
		 */
		info->txInputRate_kHz[i] = profi[i].txInputRate_Hz / 1000;
		info->outputSignaling[i] = profi[i].outputSignaling;

		if (!tx->pin_cfg)
			goto rf_enable;

		ret = adi_adrv9001_Tx_Attenuation_PinControl_Configure(phy->adrv9001,
								       tx->channel.number,
								       tx->pin_cfg);
		if (ret)
			return adrv9002_dev_err(phy);

rf_enable:
		ret = adi_adrv9001_Radio_Channel_ToState(phy->adrv9001, ADI_TX,
							 tx->channel.number, state);
		if (ret)
			return adrv9002_dev_err(phy);
	}

	return 0;
}

static const uint32_t adrv9002_init_cals_mask[16][2] = {
	/* Not a valid case. At least one channel should be enabled */
	[0] = {0, 0},
	/* tx2:0 rx2:0 tx1:0 rx1:1 */
	[1] = {0x1BE400, 0},
	/* tx2:0 rx2:0 tx1:1 rx1:0 */
	[2] = {0x1BE5F7, 0},
	/* tx2:0 rx2:0 tx1:1 rx1:1 */
	[3] = {0x1BE5F7, 0},
	/* tx2:0 rx2:1 tx1:0 rx1:0 */
	[4] = {0, 0x11E400},
	/* tx2:0 rx2:1 tx1:0 rx1:1 */
	[5] = {0x1BE400, 0x1BE400},
	/* tx2:0 rx2:1 tx1:1 rx1:0 */
	[6] = {0x1BE5F7, 0x1BE400},
	/* tx2:0 rx2:1 tx1:1 rx1:1 */
	[7] = {0x1BE5F7, 0x1BE400},
	/* tx2:1 rx2:0 tx1:0 rx1:0 */
	[8] = {0, 0x11E5F0},
	/* tx2:1 rx2:0 tx1:0 rx1:1 */
	[9] = {0x1BE400, 0x1BE5F0},
	/* tx2:1 rx2:0 tx1:1 rx1:0 */
	[10] = {0x1BE5F7, 0x1BE5F7},
	/* tx2:1 rx2:0 tx1:1 rx1:1 */
	[11] = {0x1BE5F7, 0x1BE5F7},
	/* tx2:1 rx2:1 tx1:0 rx1:0 */
	[12] = {0, 0x11E5F0},
	/* tx2:1 rx2:1 tx1:0 rx1:1 */
	[13] = {0x1BE400, 0x1BE5F0},
	/* tx2:1 rx2:1 tx1:1 rx1:0 */
	[14] = {0x1BE5F7, 0x1BE5F7},
	/* tx2:1 rx2:1 tx1:1 rx1:1 */
	[15] = {0x1BE5F7, 0x1BE5F7},
};

static int adrv9002_compute_init_cals(struct adrv9002_rf_phy *phy)
{
	const uint32_t tx_channels[ADRV9002_CHANN_MAX] = {
		ADI_ADRV9001_TX1, ADI_ADRV9001_TX2
	};
	const uint32_t rx_channels[ADRV9002_CHANN_MAX] = {
		ADI_ADRV9001_RX1, ADI_ADRV9001_RX2
	};
	const uint32_t channels[ADRV9002_CHANN_MAX] = {
		ADI_CHANNEL_1, ADI_CHANNEL_2
	};
	int i, pos = 0;

	phy->init_cals.sysInitCalMask = 0;
	phy->init_cals.calMode = ADI_ADRV9001_INIT_CAL_MODE_ALL;

	for (i = 0; i < ADRV9002_CHANN_MAX; i++) {
		struct adrv9002_tx_chan *tx = &phy->tx_channels[i];
		struct adrv9002_rx_chan *rx = &phy->rx_channels[i];

		if (ADRV9001_BF_EQUAL(phy->curr_profile->rx.rxInitChannelMask,
				      rx_channels[i])) {
#ifdef DEBUG
			printf("RX%d enabled\n", i);
#endif
			pos |= ADRV9002_RX_EN(i);
			rx->channel.power = true;
			rx->channel.enabled = true;
			rx->channel.number = channels[i];
		} else if (phy->rx2tx2 && i == ADRV9002_CHANN_1 ) {
			/*
			 * In rx2tx2 mode RX1 must be always enabled because RX2 cannot be
			 * on without RX1. On top of this, TX cannot be enabled without the
			 * corresponding RX. Hence, RX1 cannot really be disabled...
			 */
			printf("In rx2tx2 mode RX1 must be always enabled...\n");
			return -EINVAL;
		}

		if (ADRV9001_BF_EQUAL(phy->curr_profile->tx.txInitChannelMask,
				      tx_channels[i])) {
			if (!rx->channel.enabled) {
				printf("TX%d cannot be enabled while RX%d is disabled",
					i + 1, i + 1);
				return -EINVAL;
			}
#ifdef DEBUG
			printf("TX%d enabled\n", i);
#endif
			pos |= ADRV9002_TX_EN(i);
			tx->channel.power = true;
			tx->channel.enabled = true;
			tx->channel.number = channels[i];
		}

	}

	phy->init_cals.chanInitCalMask[0] = adrv9002_init_cals_mask[pos][0];
	phy->init_cals.chanInitCalMask[1] = adrv9002_init_cals_mask[pos][1];

#ifdef DEBUG
	printf("pos: %u, Chan1:%X, Chan2:%X", pos,
		phy->init_cals.chanInitCalMask[0],
		phy->init_cals.chanInitCalMask[1]);
#endif

	return 0;
}

static int adrv9002_dgpio_config(struct adrv9002_rf_phy *phy)
{
	struct adrv9002_gpio *dgpio = phy->adrv9002_gpios;
	int i, ret;

	for (i = 0; i < phy->ngpios; i++) {
		/* printf("Set dpgio: %d, signal: %d\n",
			dgpio[i].gpio.pin, dgpio[i].signal);
		*/

		ret = adi_adrv9001_gpio_Configure(phy->adrv9001,
						  dgpio[i].signal,
						  &dgpio[i].gpio);
		if (ret)
			return adrv9002_dev_err(phy);
	}

	return 0;
}

static int adrv9002_setup(struct adrv9002_rf_phy *phy,
			  adi_adrv9001_Init_t *adrv9002_init)
{
	struct adi_adrv9001_Device *adrv9001_device;
	struct adi_adrv9001_RadioCtrlInit *adrv9002_radio_init =
						adrv9002_radio_ctrl_init_get();
	struct adi_adrv9001_ResourceCfg adrv9001_resource_cfg = {
		adrv9002_init,
		adrv9002_radio_init,
		adrv9002_platform_files_get()
	};
	int ret;
	uint8_t init_cals_error = 0;
	uint8_t channel_mask = 0;
	adi_adrv9001_gpMaskArray_t gp_mask;
	adi_adrv9001_ChannelState_e init_state;

	phy->curr_profile = adrv9002_init;

	channel_mask = adrv9002_init->tx.txInitChannelMask |
		(adrv9002_init->rx.rxInitChannelMask & ALL_RX_CHANNEL_MASK);

	/* in TDD we cannot start with all ports enabled as RX/TX cannot be on at the same time */
	if (phy->curr_profile->sysConfig.duplexMode == ADI_ADRV9001_TDD_MODE)
		init_state = ADI_ADRV9001_CHANNEL_PRIMED;
	else
		init_state = ADI_ADRV9001_CHANNEL_RF_ENABLED;

	phy->adrv9001 = &phy->adrv9001_device;
	adrv9001_device = phy->adrv9001;
	phy->adrv9001->common.devHalInfo = &phy->hal;

	ret = adrv9002_compute_init_cals(phy);
	if (ret)
		return ret;

	ret = adi_adrv9001_HwOpen(adrv9001_device, adrv9002_spi_settings_get());
	if (ret)
		return adrv9002_dev_err(phy);

	adrv9002_set_loglevel(&adrv9001_device->common, ADI_HAL_LOG_ERR);

	ret = adi_adrv9001_InitAnalog(adrv9001_device, adrv9002_init,
			adrv9002_radio_init->adrv9001DeviceClockOutputDivisor);
	if (ret)
		return adrv9002_dev_err(phy);

	/* needs to be done before loading the ARM image */
	ret = adrv9002_tx_set_dac_full_scale(phy);
	if (ret)
		return ret;

	ret = adi_adrv9001_Utilities_Resources_Load(adrv9001_device,
						    &adrv9001_resource_cfg);
	if (ret)
		return adrv9002_dev_err(phy);

	ret = adi_adrv9001_Utilities_InitRadio_Load(adrv9001_device,
						    &adrv9001_resource_cfg,
						    channel_mask);
	if (ret)
		return adrv9002_dev_err(phy);

	ret = adi_adrv9001_cals_InitCals_Run(adrv9001_device, &phy->init_cals,
					     60000, &init_cals_error);
	if (ret)
		return adrv9002_dev_err(phy);

	ret = adrv9001_rx_path_config(phy, init_state);
	if (ret)
		return ret;

	ret = adrv9002_tx_path_config(phy, init_state);
	if (ret)
		return ret;

	/* unmask IRQs */
	gp_mask.gpIntMask = ~ADRV9002_IRQ_MASK;
	ret = adi_adrv9001_gpio_GpIntMask_Set(adrv9001_device,
					      ADI_ADRV9001_GPINT, &gp_mask);
	if (ret)
		return adrv9002_dev_err(phy);

	return adrv9002_dgpio_config(phy);
}

int main(void)
{
	int ret;
	struct adi_common_ApiVersion api_version;
	struct adi_adrv9001_ArmVersion arm_version;
	struct adi_adrv9001_SiliconVersion silicon_version;
	struct adrv9002_rf_phy phy;
	struct axi_adc_init rx1_adc_init = {
		"axi-adrv9002-rx-lpc",
		RX1_ADC_BASEADDR,
		ADRV9001_NUM_SUBCHANNELS,
	};

	struct axi_dac_channel  tx1_dac_channels[2];
	tx1_dac_channels[0].sel = AXI_DAC_DATA_SEL_DMA;
	tx1_dac_channels[1].sel = AXI_DAC_DATA_SEL_DMA;

	struct axi_dac_init tx1_dac_init = {
		"axi-adrv9002-tx-lpc",
		TX1_DAC_BASEADDR,
		ADRV9001_NUM_SUBCHANNELS,
		tx1_dac_channels,
	};

	struct axi_adc_init rx2_adc_init = {
		"axi-adrv9002-rx2-lpc",
		RX2_ADC_BASEADDR,
		ADRV9001_NUM_SUBCHANNELS,
	};

	struct axi_dac_channel  tx2_dac_channels[2];
	tx2_dac_channels[0].sel = AXI_DAC_DATA_SEL_DMA;
	tx2_dac_channels[1].sel = AXI_DAC_DATA_SEL_DMA;

	struct axi_dac_init tx2_dac_init = {
		"axi-adrv9002-tx2-lpc",
		TX2_DAC_BASEADDR,
		ADRV9001_NUM_SUBCHANNELS,
		tx2_dac_channels,
	};

	struct axi_dmac_init rx1_dmac_init = {
		"rx_dmac",
		RX1_DMA_BASEADDR,
		DMA_DEV_TO_MEM,
		0
	};

	struct axi_dmac_init tx1_dmac_init = {
		"tx_dmac",
		TX1_DMA_BASEADDR,
		DMA_MEM_TO_DEV,
		DMA_CYCLIC,
	};

#ifndef ADRV9002_RX2TX2
	struct axi_dmac_init rx2_dmac_init = {
		"rx_dmac",
		RX2_DMA_BASEADDR,
		DMA_DEV_TO_MEM,
		0
	};

	struct axi_dmac_init tx2_dmac_init = {
		"tx_dmac",
		TX2_DMA_BASEADDR,
		DMA_MEM_TO_DEV,
		DMA_CYCLIC,
	};
#endif

	Xil_ICacheEnable();
	Xil_DCacheEnable();

	printf("Hello\n");

	memset(&phy, 0, sizeof(struct adrv9002_rf_phy));

#if defined(ADRV9002_RX2TX2)
	phy.rx2tx2 = true;
#endif

	ret = adrv9002_setup(&phy, adrv9002_init_get());
	if (ret)
		return ret;

	adi_adrv9001_ApiVersion_Get(phy.adrv9001, &api_version);
	adi_adrv9001_arm_Version(phy.adrv9001, &arm_version);
	adi_adrv9001_SiliconVersion_Get(phy.adrv9001, &silicon_version);

	printf("%s Rev %d.%d, Firmware %u.%u.%u.%u API version: %u.%u.%u successfully initialized\n",
		"ADRV9002", silicon_version.major, silicon_version.minor,
		arm_version.majorVer, arm_version.minorVer,
		arm_version.maintVer, arm_version.rcVer, api_version.major,
		api_version.minor, api_version.patch);

	/* Initialize the ADC/DAC cores */
	ret = axi_adc_init(&phy.rx1_adc, &rx1_adc_init);
	if (ret) {
		printf("axi_adc_init() failed with status %d\n", ret);
		goto error;
	}

	ret = axi_dac_init(&phy.tx1_dac, &tx1_dac_init);
	if (ret) {
		printf("axi_dac_init() failed with status %d\n", ret);
		goto error;
	}

	ret = axi_adc_init(&phy.rx2_adc, &rx2_adc_init);
	if (ret) {
		printf("axi_adc_init() failed with status %d\n", ret);
		goto error;
	}

	ret = axi_dac_init(&phy.tx2_dac, &tx2_dac_init);
	if (ret) {
		printf("axi_dac_init() failed with status %d\n", ret);
		goto error;
	}

	/* Post AXI DAC/ADC setup, digital interface tuning */
	ret = adrv9002_post_setup(&phy);
	if (ret) {
		printf("adrv9002_post_setup() failed with status %d\n", ret);
		goto error;
	}

	/* TODO: investigate why this is needed, it shouldn't be needed. */
	ret = adi_adrv9001_Radio_Channel_ToState(phy.adrv9001, ADI_RX,
						 ADI_CHANNEL_1, ADI_ADRV9001_CHANNEL_PRIMED);
	ret = adi_adrv9001_Radio_Channel_ToState(phy.adrv9001, ADI_RX,
						 ADI_CHANNEL_1, ADI_ADRV9001_CHANNEL_RF_ENABLED);
	ret = adi_adrv9001_Radio_Channel_ToState(phy.adrv9001, ADI_RX,
						 ADI_CHANNEL_2, ADI_ADRV9001_CHANNEL_PRIMED);
	ret = adi_adrv9001_Radio_Channel_ToState(phy.adrv9001, ADI_RX,
						 ADI_CHANNEL_2, ADI_ADRV9001_CHANNEL_RF_ENABLED);

	/* Initialize the AXI DMA Controller cores */
	ret = axi_dmac_init(&phy.tx1_dmac, &tx1_dmac_init);
	if (ret) {
		printf("axi_dmac_init() failed with status %d\n", ret);
		goto error;
	}

	ret = axi_dmac_init(&phy.rx1_dmac, &rx1_dmac_init);
	if (ret) {
		printf("axi_dmac_init() failed with status %d\n", ret);
		goto error;
	}
#ifndef ADRV9002_RX2TX2
	ret = axi_dmac_init(&phy.tx2_dmac, &tx2_dmac_init);
	if (ret) {
		printf("axi_dmac_init() failed with status %d\n", ret);
		goto error;
	}

	ret = axi_dmac_init(&phy.rx2_dmac, &rx2_dmac_init);
	if (ret) {
		printf("axi_dmac_init() failed with status %d\n", ret);
		goto error;
	}
#endif

#ifdef DAC_DMA_EXAMPLE
	axi_dac_load_custom_data(phy.tx1_dac, sine_lut_iq,
				 ARRAY_SIZE(sine_lut_iq),
				 DAC1_DDR_BASEADDR);
#ifndef ADRV9002_RX2TX2
	axi_dac_load_custom_data(phy.tx2_dac, sine_lut_iq,
				 ARRAY_SIZE(sine_lut_iq),
				 DAC2_DDR_BASEADDR);
#endif
	Xil_DCacheFlush();

	axi_dmac_transfer(phy.tx1_dmac, DAC1_DDR_BASEADDR, sizeof(sine_lut_iq));
#ifndef ADRV9002_RX2TX2
	axi_dmac_transfer(phy.tx2_dmac, DAC2_DDR_BASEADDR, sizeof(sine_lut_iq));
#endif

	mdelay(1000);

	/* Transfer 16384 samples from ADC to MEM */
	axi_dmac_transfer(phy.rx1_dmac,
			  ADC1_DDR_BASEADDR,
			  16384 * /* nr of samples */
			  ADRV9001_NUM_CHANNELS * /* nr of channels */
			  2 /* bytes per sample */);
	Xil_DCacheInvalidateRange(ADC1_DDR_BASEADDR,
				  16384 * /* nr of samples */
				  ADRV9001_NUM_CHANNELS * /* nr of channels */
				  2 /* bytes per sample */);
#ifndef ADRV9002_RX2TX2
	axi_dmac_transfer(phy.rx2_dmac,
			  ADC2_DDR_BASEADDR,
			  16384 * /* nr of samples */
			  ADRV9001_NUM_CHANNELS * /* nr of channels */
			  2 /* bytes per sample */);
	Xil_DCacheInvalidateRange(ADC2_DDR_BASEADDR,
				  16384 * /* nr of samples */
				  ADRV9001_NUM_CHANNELS * /* nr of channels */
				  2 /* bytes per sample */);
#endif
#endif

#ifdef IIO_SUPPORT
        printf("The board accepts libiio clients connections through the serial backend.\n");

	struct iio_axi_adc_init_param iio_axi_adc1_init_par = {
		.rx_adc = phy.rx1_adc,
		.rx_dmac = phy.rx1_dmac,
		.adc_ddr_base = ADC1_DDR_BASEADDR,
		.dcache_invalidate_range = (void (*)(uint32_t, uint32_t))Xil_DCacheInvalidateRange,
	};

	struct iio_axi_adc_init_param iio_axi_adc2_init_par = {
		.rx_adc = phy.rx2_adc,
#ifndef ADRV9002_RX2TX2
		.rx_dmac = phy.rx2_dmac,
#else
		.rx_dmac = phy.rx1_dmac,
#endif
		.adc_ddr_base = ADC2_DDR_BASEADDR,
		.dcache_invalidate_range = (void (*)(uint32_t, uint32_t))Xil_DCacheInvalidateRange,
	};

	struct iio_axi_dac_init_param iio_axi_dac1_init_par = {
		.tx_dac = phy.tx1_dac,
		.tx_dmac = phy.tx1_dmac,
		.dac_ddr_base = DAC1_DDR_BASEADDR,
		.dcache_flush_range = (void (*)(uint32_t, uint32_t))Xil_DCacheFlushRange,
	};

	struct iio_axi_dac_init_param iio_axi_dac2_init_par = {
		.tx_dac = phy.tx2_dac,
#ifndef ADRV9002_RX2TX2
		.tx_dmac = phy.tx2_dmac,
#else
		.tx_dmac = phy.tx1_dmac,
#endif
		.dac_ddr_base = DAC2_DDR_BASEADDR,
		.dcache_flush_range = (void (*)(uint32_t, uint32_t))Xil_DCacheFlushRange,
	};

        ret = iio_server_init(&iio_axi_adc1_init_par,
			      &iio_axi_adc2_init_par,
			      &iio_axi_dac1_init_par,
			      &iio_axi_dac2_init_par);
#endif
        printf("Bye\n");

error:
	adi_adrv9001_HwClose(phy.adrv9001);
	axi_adc_remove(phy.rx1_adc);
	axi_dac_remove(phy.tx1_dac);
	axi_adc_remove(phy.rx2_adc);
	axi_dac_remove(phy.tx2_dac);
	axi_dmac_remove(phy.rx1_dmac);
	axi_dmac_remove(phy.tx1_dmac);
	axi_dmac_remove(phy.rx2_dmac);
	axi_dmac_remove(phy.tx2_dmac);
	return ret;
}
