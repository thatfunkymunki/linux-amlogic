# STB 2.4 host profile (P3i T4c) and the vendor 4.9 GCC interface.
DOVI_MODULE_NAME := dovi
CROSS_COMPILE ?= aarch64-linux-gnu-
DOVI_KERNEL_OPTIONS := CROSS_COMPILE=$(CROSS_COMPILE)
ccflags-y += -I$(src) -I$(srctree)/drivers/amlogic/media/enhancement/amdolby_vision
ccflags-y += -std=gnu11 -Wno-declaration-after-statement -fno-builtin
