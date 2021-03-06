#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

M3DEBUG ?= m3debug

ifneq ($(CPU),cortexm4f)
        $(error "stm32f4xx cplatform only supports Cortex-M4F CPUs")
endif

DELIVERABLES = $(APP).full.bin $(APP).update.bin

FLAGS += -I. -fno-unwind-tables -fstack-reuse=all -ffunction-sections -fdata-sections
FLAGS += -Wl,--gc-sections
FLAGS += -Iinc/platform/$(PLATFORM)/cmsis

APPFLAGS += -msingle-pic-base -mpic-register=r9 -nostartfiles -fpic -shared

#platform bootloader
SRCS += src/platform/$(PLATFORM)/bl.c

#platform runtime
SRCS +=  src/platform/$(PLATFORM)/crt_$(PLATFORM).c

#platform drivers
SRCS += src/platform/$(PLATFORM)/platform.c \
	src/platform/$(PLATFORM)/usart.c \
	src/platform/$(PLATFORM)/gpio.c \
	src/platform/$(PLATFORM)/pwr.c \
	src/platform/$(PLATFORM)/i2c.c \
	src/platform/$(PLATFORM)/exti.c \
	src/platform/$(PLATFORM)/syscfg.c \
	src/platform/$(PLATFORM)/spi.c \
	src/platform/$(PLATFORM)/rtc.c \
	src/platform/$(PLATFORM)/mpu.c \
	src/platform/$(PLATFORM)/dma.c \
	src/platform/$(PLATFORM)/crc.c \
	src/platform/$(PLATFORM)/hostIntf.c \
	src/platform/$(PLATFORM)/apInt.c \

#extra deps
DEPS += $(wildcard inc/platform/$(PLATFORM)/*.h)
DEPS += $(wildcard inc/platform/$(PLATFORM)/cmsis/*.h)

#platform flags
PLATFORM_HAS_HARDWARE_CRC = true
FLAGS += -DPLATFORM_HW_VER=0

#platform-specific rules

%.full.bin : %.elf
	$(OBJCOPY) -j.bl -j .data -j .text -j .eedata -I elf32-littlearm -O binary $< $@

%.update.bin : %.elf
	$(OBJCOPY) -j .data -j .text -I elf32-littlearm -O binary $< $@

flash: $(APP).full.bin
	echo -e "attach\nscript misc/platform/$(PLATFORM)/m3debug.script\nflash $(APP).full.bin 08000000\n" | $(M3DEBUG)
