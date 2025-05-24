CC=gcc
CC_ARGS=-c -ffreestanding -nostdlib -fno-builtin -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -mcmodel=large -fno-stack-protector -fomit-frame-pointer -O0 -std=c99 -shared -Tfxe.ld
KEYGEN=tr -dc A-Za-z0-9 </dev/urandom | head -c 64
AS=nasm
AS_ARGS=-fbin

all:
	$(CC) $(CC_ARGS) fxebtldr.c -o fxebtldr.o 
	$(KEYGEN) > key.bin
	$(AS) $(AS_ARGS) main.s -o fxebtldr.bin
	rm key.bin