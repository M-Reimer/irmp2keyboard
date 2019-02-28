#!/usr/bin/bash

# Exit with error as soon as any command fails
set -e

#
# Get tar file
#

wget 'https://www.mikrocontroller.net/svnbrowser/irmp/?view=tar' -O irmp.tar.gz

#
# Remove irmp source directories
#

rm -rf irmp2keyboard/src/irmp
rm -rf irmpdump/src/irmp

#
# Unextract the new IRMP source to our first source directory
#

tar -xf irmp.tar.gz -C irmp2keyboard/src
rm irmp.tar.gz

# Switch to the IRMP source
pushd irmp2keyboard/src/irmp >/dev/null

#
# Some files have to go as Arduino will just try to compile every C file
#

rm irmp-main-*.c
rm irmp-main-*.cpp
rm irmpextlog.c
rm irsnd*

#
# Some stuff is not needed for our project (and so shouldn't be in our git)
#

rm -r IR-Data
rm -r examples
rm -r pic
rm *.aps
rm *.lnx
rm *.exe

#
# Do some preconfiguration
#

# Push F_INTERRUPTS to maximum
sed -i 's/\(F_INTERRUPTS\s*\) 15000/\1 20000/' irmpconfig.h

# Some protocol (p)reconfiguration
sed -i 's/\(IRMP_SUPPORT_JVC_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_NEC16_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_NEC42_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_MATSUSHITA_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_DENON_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_RC5_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_RC6_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_IR60_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_GRUNDIG_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_SIEMENS_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_NOKIA_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_RCMM_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h
sed -i 's/\(IRMP_SUPPORT_FDC_PROTOCOL\s*\) 0/\1 1/' irmpconfig.h

sed -i 's/\(IRMP_SUPPORT_RCII_PROTOCOL\s*\) 1/\1 0/' irmpconfig.h

# Switch port from PB6 to PB4 (PB6 is the oscillator on Atmega328)
# Arduino Nano/Uno:           D12
# Arduino Pro Micro/Leonardo: D8
sed -i 's/\(#\s*define IRMP_BIT_NUMBER\s*\) 6/\1 4/' irmpconfig.h

# We want to use external logging
sed -i 's/\(IRMP_EXT_LOGGING\s*\) 0/\1 1/' irmpconfig.h

#
# Arduino support in IRMP is broken (checks for ARDUINO but then uses
# Teensy-only API). So just hard-disable it completely.
#

#sed -i 's/ARDUINO/xARDUINOx/g' irmp.c
patch -p1 -i ../../../irmp-add-arduino-ext-logging.patch

# Finally copy our new IRMP version over to the second source directory
popd >/dev/null
cp -r irmp2keyboard/src/irmp irmpdump/src

echo "All done!"
