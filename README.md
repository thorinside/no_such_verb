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

## Using

By default the make file will compile the firmware so that it can be placed on an SD card and placed in the Patch.init(), you will have to perform a `make program-boot` on your device to install the bootloader. This way you can manage firmwares on the SD and not have to hook the module up to USB.
