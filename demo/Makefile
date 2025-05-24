CC=gcc
LD=ld
AS=nasm

CC_ARGS=-c -ffreestanding -nostdlib -fno-builtin -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -mcmodel=large -fno-stack-protector -fomit-frame-pointer -fno-pic -O0 -std=c99
LD_ARGS=-ffreestanding -shared -T../FXE/fxe.ld
AS_ARGS=-felf64

all:
	$(AS) $(AS_ARGS) kernel.s -o kernel.o
	$(CC) $(CC_ARGS) kernel.c -o kernel2.o
	$(LD) $(LD_ARGS) kernel.o kernel2.o -o kernel.fxe

	rm kernel.o kernel2.o