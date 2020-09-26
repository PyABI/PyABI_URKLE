#!/usr/bin/env python3
# encoding: utf-8

from distutils.core import setup, Extension

names = ['PyABI_URKLE.cpp']

names.append('sqlite3.c')

abi_module = Extension('PyABI_URKLE_pyd', sources = names)

setup(name='PyABI_URKLE_pyd',
      version='0.0.42',
      description='C++ PyABI_URKLE (https://github.com/chjj/liburkel)',
      ext_modules=[abi_module])
