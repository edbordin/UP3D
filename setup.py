#!/usr/bin/env python3
# encoding: utf-8

from distutils.core import setup, Extension

pyUP3D_module = Extension('pyUP3D',
                    sources = ['csrc/pyUP3D.c',
                        'csrc/UP3DCOMMON/up3dcomm.c', 'csrc/UP3DCOMMON/up3d.c', 'csrc/UP3DCOMMON/up3ddata.c'],
                    define_macros = [('_BSD_SOURCE', '__USE_MISC')],
                    include_dirs = ['csrc/UP3DCOMMON'],
                    libraries = ['usb-1.0', 'pthread', 'm'],
                    extra_compile_args=['-std=c99'])

setup(name='pyUP3D',
      version='0.1.0',
      description='UP3D USB Control module written in C',
      ext_modules=[pyUP3D_module])