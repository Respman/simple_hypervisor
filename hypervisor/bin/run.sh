#! /usr/bin/env bash

cd ../mbr/
fasm ./mbr.asm
cp ./mbr.bin ../bin/
cd ../bin/

cd ../hypervisor/
make
cp ./hypervisor.hex ../bin/
cd ../bin/

cd ../loader64/
fasm ./loader64.asm
cp ./loader64.bin ../bin/
cd ../bin/

rm ./image.bin

dd if=mbr.bin of=image.bin bs=512 seek=0
dd if=loader64.bin of=image.bin bs=512 seek=1
dd if=hypervisor.hex of=image.bin bs=512 seek=6

qemu-system-x86_64 -hda ./image.bin -hdb ../../test_OS/image.bin -serial stdio -m 2G -cpu host -enable-kvm