AS=nasm
CC=gcc
LD=ld
CC_ARGS=-c -ffreestanding -nostdlib -fno-builtin -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -m16 -fno-stack-protector -fomit-frame-pointer -fno-pic -O0 -std=c99
LD_ARGS=-ffreestanding -shared -Tfxebtldr.ld
EMU=qemu-system-x86_64
EMU_ARGS=-cpu max -m 512M -display gtk,zoom-to-fit=on,gl=on

all:
	tr -dc A-Za-z0-9 </dev/urandom | head -c 64 > key.bin
	$(AS) boot.asm -o boot.bin -fbin
	$(AS) main.asm -o main.o -felf
	$(CC) -c main.c -o main2.o $(CC_ARGS)
	$(LD) main.o main2.o -o fxebtldr.fxb $(LD_ARGS)
	cat boot.bin fxebtldr.fxb > fxebtldr.img
	truncate -s 32768 fxebtldr.img

run:
	$(EMU) -hda fxebtldr.img $(EMU_ARGS)