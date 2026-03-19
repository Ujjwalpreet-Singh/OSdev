#!/bin/bash

qemu-system-i386 -vga std -drive format=raw,file=build/main.img
