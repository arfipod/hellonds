# SPDX-License-Identifier: CC0-1.0
#
# Nintendo DSi + TWiLight Menu++ HTTPS REST demo using libcurl.
# Requires:
#   wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib blocksds-mbedtls blocksds-libcurl

BLOCKSDS       ?= /opt/wonderful/thirdparty/blocksds/core
BLOCKSDSEXT    ?= /opt/wonderful/thirdparty/blocksds/external
WONDERFUL_TOOLCHAIN ?= /opt/wonderful

MBEDTLS_FOUND := $(shell find $(BLOCKSDSEXT)/mbedtls -name libmbedtls.a 2> /dev/null)
LIBCURL_FOUND := $(shell find $(BLOCKSDSEXT)/libcurl -name libcurl.a 2> /dev/null)
ZLIB_FOUND    := $(shell find $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/arm-none-eabi/lib/ -name libz.a 2> /dev/null)

ifneq ($(strip $(MBEDTLS_FOUND)),)
  ifneq ($(strip $(LIBCURL_FOUND)),)
    ifneq ($(strip $(ZLIB_FOUND)),)
      PACKAGES_FOUND := true
    endif
  endif
endif

ifneq ($(strip $(PACKAGES_FOUND)),true)

.PHONY: all clean

all:
	@echo "Error: HTTPS dependencies missing. Run:"
	@echo "  wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib blocksds-mbedtls blocksds-libcurl"

clean:
	@echo "  CLEAN"

else

NAME          := dsi_rest_https_curl
GAME_TITLE    := DSi REST HTTPS
GAME_SUBTITLE := libcurl DSWiFi
GAME_AUTHOR   := arfipod + ChatGPT

ARM7ELF := $(BLOCKSDS)/sys/arm7/main_core/arm7_dswifi.elf

LIBS    := -ldswifi9 -lnds9 -lmbedcrypto -lmbedtls -lmbedx509 -lcurl -lz
LIBDIRS := $(BLOCKSDS)/libs/dswifi \
           $(BLOCKSDS)/libs/libnds \
           $(BLOCKSDSEXT)/mbedtls \
           $(BLOCKSDSEXT)/libcurl

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile

endif
