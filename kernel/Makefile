C_SOURCES=$(shell find -name '*.c')
C_OBJECTS=$(C_SOURCES:.c=.obj)

ASM_SOURCES=$(shell find -name '*.s')
ASM_OBJECTS=$(ASM_SOURCES:.s=.o)

CC=gcc
LD=ld
AS=nasm

CC_ARGS=-c -ffreestanding -nostdlib -fno-builtin -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -mcmodel=large -fno-stack-protector -fomit-frame-pointer -fno-pic -O0 -std=c99
LD_ARGS=-ffreestanding -shared -Tfxe.ld
AS_ARGS=-felf64

all:
	for ASM_SOURCE in $(ASM_SOURCES); do \
        $(AS) $(AS_ARGS) $$ASM_SOURCE; \
	done

	$(CC) $(CC_ARGS) $(C_SOURCES) -o $(C_OBJECTS) 
	$(LD) $(LD_ARGS) $(ASM_OBJECTS) $(C_OBJECTS) -o kernel.fxe

	rm $(C_OBJECTS) $(ASM_OBJECTS)
