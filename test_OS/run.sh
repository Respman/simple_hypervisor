#! /usr/bin/env bash

cd ./OS/
fasm ./mbr.asm
fasm ./kernel.asm
cd ../
mv ./OS/mbr.bin ./mbr.bin
mv ./OS/kernel.bin ./kernel.bin


dd if=/dev/null of=image.bin bs=512 count=4 
dd if=mbr.bin of=image.bin bs=512 count=1 seek=0
dd if=kernel.bin of=image.bin bs=512 seek=1 

qemu-system-x86_64 -drive file=./image.bin,index=0,media=disk,format=raw -serial stdio -cpu host -enable-kvm

rm ./mbr.bin
rm ./kernel.bin
