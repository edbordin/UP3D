#!/usr/bin/env python3
# encoding: utf-8

from distutils.core import setup, Extension

pyUP3D_com_module = Extension('pyUP3D_com',
                    sources = ['csrc/pyUP3D_com.c',
                        'csrc/UP3DCOMMON/up3dcomm.c', 'csrc/UP3DCOMMON/up3d.c', 'csrc/UP3DCOMMON/up3ddata.c',
                        'csrc/UP3DCOMMON/printLink.c'],
                    define_macros = [('_BSD_SOURCE', '__USE_MISC')],
                    include_dirs = ['csrc/UP3DCOMMON'],
                    libraries = ['usb-1.0', 'pthread', 'm'],
                    extra_compile_args=['-std=c99'])

setup(name='pyUP3D',
    version='0.1.9',
    py_modules=['pyup3d'],
    # install_requires=['libusb'],
    description='UP3D USB Control module written in C',
    ext_modules=[pyUP3D_com_module])