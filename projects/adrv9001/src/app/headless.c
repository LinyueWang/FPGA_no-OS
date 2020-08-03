#include <stdio.h>
#include <string.h>
#include "error.h"

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

/* init data stuff */
extern struct adi_adrv9001_SpiSettings *adrv9002_spi_settings_get(void);
extern struct adi_adrv9001_Init *adrv9002_init_get(void);
extern struct adi_adrv9001_RadioCtrlInit *adrv9002_radio_ctrl_init_get(void);
extern struct adi_adrv9001_PlatformFiles *adrv9002_platform_files_get(void);
extern void adrv9002_cmos_default_set(void);
extern adi_adrv9001_Init_t adrv9002_init_lvds;
extern adi_adrv9001_Init_t adrv9002_init_cmos;

static void adrv9002_cleanup(struct adrv9002_rf_phy *phy)
{
	int i;

	for (i = 0; i < ADRV9002_CHANN_MAX; i++) {
		memset(&phy->rx_channels[i].channel, 0,
		       sizeof(struct adrv9002_chan));

		memset(&phy->tx_channels[i].channel, 0,
		       sizeof(struct adrv9002_chan));
	}

	memset(&phy->adrv9001->devStateInfo, 0,
	       sizeof(phy->adrv9001->devStateInfo));
}

static int adrv9002_setup(struct adrv9002_rf_phy *phy,
			  adi_adrv9001_Init_t *adrv9002_init)
{
	int ret;
	struct adi_adrv9001_Device *adrv9001_device = phy->adrv9001;

	phy->adrv9001 = &phy->adrv9001_device;

	ret = adi_adrv9001_HwOpen(adrv9001_device, adrv9002_spi_settings_get());
	if (ret)
		return ret;

	return ret;
}

int main(void)
{
	int ret;
	struct adrv9002_rf_phy phy;
	printf("Hello\n");

	adrv9002_cleanup(&phy);
	ret = adrv9002_setup(&phy, adrv9002_init_get());
	if (ret)
		return ret;

success:
	printf("Bye\n");
	return SUCCESS;
}