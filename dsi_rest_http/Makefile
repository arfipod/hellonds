# SPDX-License-Identifier: CC0-1.0
#
# Nintendo DSi + TWiLight Menu++ plain HTTP REST demo.
# Requires: BlocksDS core libraries only.

BLOCKSDS ?= /opt/wonderful/thirdparty/blocksds/core

NAME          := dsi_rest_http
GAME_TITLE    := DSi REST HTTP
GAME_SUBTITLE := DSWiFi sockets
GAME_AUTHOR   := arfipod + ChatGPT

ARM7ELF := $(BLOCKSDS)/sys/arm7/main_core/arm7_dswifi.elf

LIBS    := -ldswifi9 -lnds9
LIBDIRS := $(BLOCKSDS)/libs/dswifi \
           $(BLOCKSDS)/libs/libnds

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile
