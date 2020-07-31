################################################################################
#									       #
#     Shared variables:							       #
#	- PROJECT							       #
#	- DRIVERS							       #
#	- INCLUDE							       #
#	- PLATFORM_DRIVERS						       #
#	- NO-OS								       #
#									       #
################################################################################
SRCS += $(DRIVERS)/platform/xilinx/gpio.c
SRCS += $(PROJECT)/src/hal/no_os_platform.c				\
	$(PROJECT)/src/app/headless.c
INCS +=	$(INCLUDE)/spi.h						\
	$(INCLUDE)/gpio.h						\
	$(DRIVERS)/platform/xilinx/gpio_extra.h				\
	$(INCLUDE)/error.h						\
	$(INCLUDE)/delay.h						\
	$(INCLUDE)/util.h
INCS += $(PROJECT)/src/hal/parameters.h					\
	$(PROJECT)/src/hal/adi_platform.h				\
	$(PROJECT)/src/hal/adi_platform_types.h
