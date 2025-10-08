/*
 * ======================================================================================================================
 * SDU.h - Add SDUBoot Support for UPDATE.BIN on SD card
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * No Change Needed ~/Library/Arduino15/packages/adafruit/hardware/samd/1.7.16/variants/feather_m0/linker_scripts/gcc
 *
 * Replace Files in ~/Library/Arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/SDU/extras/SDUBoot with
 *                  ~/Documents/Arduino/3D-PAWS/3D-PAWS-Feather-LoRa-Remote/SDUBoot/
 *
 * Change directory to ~/Library/Arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/SDU/extras/SDUBoot
 * Then run ./build.sh
 *
 * Copy ~/Library/Arduino15/packages/adafruit/hardware/samd/1.7.16/libraries/SDU/src/boot/featherm0.h to
 *      ~/Documents/Arduino/3D-PAWS/3D-PAWS-Feather-LoRa-Remote/FeatherLoRaRemote/boot/
 *
 * Include this file at top of your code and compile
 * ======================================================================================================================
 */

__attribute__ ((section(".sketch_boot")))
unsigned char sduBoot[0x6000] = {
    #include "featherm0.h"
};
