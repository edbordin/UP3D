#! /bin/bash

gcc -Os -w -std=c99 -D_BSD_SOURCE -I../UP3DCOMMON/ -o up3dstatus ../UP3DCOMMON/up3dcomm.c ../UP3DCOMMON/printLink.c ../UP3DCOMMON/up3d.c ../UP3DCOMMON/up3ddata.c ../UP3DCOMMON/up3dconf.c upstatus.c -lusb-1.0 -lpthread -lm
