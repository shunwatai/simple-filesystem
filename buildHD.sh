#!/bin/bash
#rm ./HD
dd if=/dev/zero of=./HD bs=4096 count=28160
clang mkfs_t.c -o mkfs_t && ./mkfs_t ./HD
