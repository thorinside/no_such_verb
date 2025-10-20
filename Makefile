
# Project Name
TARGET = no_such_verb

USE_DAISYSP_LGPL = 1

APP_TYPE=BOOT_QSPI
# NOTE: Program is too large (549KB) for BOOT_SRAM mode (480KB limit)
# The reverb object alone is 387KB due to delay buffers
# BOOT_QSPI mode means persistent settings CANNOT work (hardware limitation)
# QSPI writes are blocked while code executes from QSPI
# Alternative: Use SD card or external EEPROM for settings storage

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP
USE_FATFS = 1
LDFLAGS += -u _printf_float

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
