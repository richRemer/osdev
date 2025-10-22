esp-size = 44

default: image

image: disk.img

clean:
	rm -fr *.img *.efi *.bin

disk.img: esp.img
	@rm -fr $@
	dd if=/dev/zero of=$@ bs=1M count=48
	/sbin/parted $@ mktable gpt
	/sbin/parted $@ mkpart esp fat32 1MiB 45MiB

esp.img: bootx64.efi kernel.bin
	@rm -fr $@
	mkesp $@ --size 44
	mcopy -i $@ $< ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $@ $(word 2, $^) ::/KERNEL.BIN

bootx64.efi: ../bootx64.efi/bootx64.efi
	cp $< $@

kernel.bin: ../kernel.bin/kernel.bin
	cp $< $@

../bootx64.efi/bootx64.efi:
	make -C $(@D)

../kernel.bin/kernel.bin:
	make -C $(@D)

.PHONY: default image clean
