# Reverb for Patch.init()

This is a combination of a few DaisySP algorithms and integration with the Patch.init() hardware. Inspired by Benjie Jiao's MiniVerb and taking it into a slightly darker dimension with some added square noise and filtering.

Controls:

CV1/5 - Send Level
CV2/6 - Jitter/Overdrive
CV3/7 - Feedback/HP Filter
CV4/8 - Reverb LP Filter

Button - Turn overdrive on or off (shows on LED)
Toggle - Insert noise before or after the reverb UP is before, Down is after

Audio In and Out per usual

## State Persistence

The No Such Verb module automatically saves the overdrive button state to QSPI flash memory. When powered on, it recovers the previous state automatically, so the LED and overdrive setting match what you had before powering off.

**Important:** The Daisy Patch.init() bootloader loads firmware from SD card during startup. This process requires careful QSPI coordination to prevent data corruption. The firmware implements a 3000ms delay after hardware initialization to allow the bootloader to complete before accessing QSPI storage.

For technical details on bootloader/QSPI timing and memory layout, see [docs/architecture.md](docs/architecture.md#bootloader--qspi-coordination).

## Using

By default the make file will compile the firmware so that it can be placed on an SD card and placed in the Patch.init(), you will have to perform a `make program-boot` on your device to install the bootloader. This way you can manage firmwares on the SD and not have to hook the module up to USB.
