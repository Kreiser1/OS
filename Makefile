EMU=qemu-system-x86_64
EMU_ARGS=-cpu max -m 512M -display gtk,zoom-to-fit=on,gl=on

all:
	$(MAKE) -C FXE
	$(MAKE) -C kernel
	cat FXE/fxebtldr.img kernel/kernel.fxe > drive.bin
	truncate -s %2M drive.bin
	qemu-img convert -f raw -O vpc drive.bin drive.vhd
	rm drive.bin

.PHONY: demo
demo:
	$(MAKE) -C FXE
	$(MAKE) -C demo
	cat FXE/fxebtldr.img demo/kernel.fxe > drive.bin
	truncate -s %2M drive.bin
	qemu-img convert -f raw -O vpc drive.bin drive.vhd
	rm drive.bin
	$(MAKE) run

run:
	$(EMU) $(EMU_ARGS) -hda drive.vhd