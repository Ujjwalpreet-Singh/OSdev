#!/bin/bash

make
qemu-system-i386 -drive format=raw,file=build/main.img
