#!/bin/bash

if [ -z "$CC" ]; then
CC=gcc
fi

if [ -z "$STRIP" ]; then
STRIP=strip
fi

if [[ "$OSTYPE" == "msys" ]]; then

$CC -Os -std=c99 -static \
    -D_BSD_SOURCE \
    -o parse.exe parse.c

$STRIP parse.exe

$CC -Os -std=c99 -static \
    -D_BSD_SOURCE \
    -I../UP3DCOMMON/ \
    -o up3dconvg.exe ../UP3DCOMMON/up3dconf.c convg.c

$STRIP up3dconvg.exe

elif [[ "$OSTYPE" == "darwin"* ]]; then

# note: OSX: use homebrew, install with brew libusb-devel + libncurses-devel and compile

$CC -Os -Wall \
    -framework IOKit \
    -framework CoreFoundation \
    -lobjc \
    -o parse parse.c

$STRIP parse

$CC -Os -Wall \
    -framework IOKit \
    -framework CoreFoundation \
    -I../UP3DCOMMON/ \
    -lobjc \
    -o up3dconvg ../UP3DCOMMON/up3dconf.c convg.c

$STRIP up3dconvg

elif [[ "$OSTYPE" == "linux-gnu" ]]; then

# note: LINUX: install libudev-dev libusb-1.0.0-dev + libncurses-dev and compile

$CC -Os -std=c99 \
    -D_BSD_SOURCE \
    -I../UP3DCOMMON/ \
    -o parse parse.c \
    -lpthread -lm


$STRIP parse

$CC -Os -std=c99 \
    -D_BSD_SOURCE \
    -I../UP3DCOMMON/ \
    -o up3dconvg ../UP3DCOMMON/up3dconf.c convg.c \
    -lpthread -lm

$STRIP up3dconvg

fi