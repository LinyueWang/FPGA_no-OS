# Uncomment to use the desired platform
PLATFORM = xilinx
# PLATFORM = altera
# PLATFORM = aducm3029

CFLAGS = -DSI_REV_B0 \
	-DADI_DYNAMIC_PROFILE_LOAD \
	-DADI_COMMON_VERBOSE=1 \
	-DADI_ADRV9001_ARM_VERBOSE \
	-DADI_VALIDATE_PARAMS \
	-DADRV9002_RX2TX2 \
	-DDAC_DMA_EXAMPLE

include ../../tools/scripts/generic_variables.mk

include src.mk

include ../../tools/scripts/generic.mk
