PROJECT_NAME = $(shell basename "$(realpath ./)")

APPLICATION_SRCS = $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += softdevice_handler.c
APPLICATION_SRCS += ble_advdata.c
APPLICATION_SRCS += ble_conn_params.c
APPLICATION_SRCS += app_timer.c
APPLICATION_SRCS += app_error.c
APPLICATION_SRCS += ble_srv_common.c
APPLICATION_SRCS += app_util_platform.c
APPLICATION_SRCS += nrf_drv_common.c

APPLICATION_SRCS += nrf_drv_clock.c
APPLICATION_SRCS += ble_dis.c
APPLICATION_SRCS += led.c
APPLICATION_SRCS += nrf_drv_spi.c 
APPLICATION_SRCS += simple_ble.c



APPLICATION_SRCS += simple_adv.c
APPLICATION_SRCS += eddystone.c
APPLICATION_SRCS += fm25l04b.c




DEVICE = NRF51

ifndef TARGET
    TARGET = HERALD
endif

TARGET_UPPER = $(shell echo $(TARGET) | tr a-z A-Z)
TARGET_LOWER = $(shell echo $(TARGET) | tr A-Z a-z)
BOARD = BOARD_$(TARGET_UPPER)

APPLICATION_SRCS += $(TARGET_LOWER).c

SOFTDEVICE_MODEL = s130

SDK_VERSION = 12

LIBRARY_PATHS += /home/cynthia/Herald/software/include
SOURCE_PATHS += /home/cynthia/Herald/software/src

#CFLAGS += -Os
#GDB_PORT_NUMBER = 2331

NRF_BASE_PATH ?= /home/cynthia/Herald/software/nrf5x-base
include $(NRF_BASE_PATH)/make/Makefile
