include build.conf

esp-size = 44

default: image

image: disk.img

vdi: theos.vdi

clean:
	rm -fr *.img *.efi *.bin *.vdi

distclean: clean
	rm -fr *.conf

attach: theos.vdi
ifndef vm
	$(error project must first be configured)
endif
	VBoxManage storageattach $(vm) --storagectl IDE \
		--device 0 --port 0 --type hdd --medium $<

run: attach
ifndef vm
	$(error project must first be configured)
endif
	VBoxManage startvm $(vm)

theos.vdi: disk.img
	@rm -fr $@
	VBoxManage convertfromraw $< $@

disk.img: esp.img
	@rm -fr $@
	dd if=/dev/zero of=$@ bs=1M count=48
	/sbin/parted $@ mktable gpt
	/sbin/parted $@ mkpart esp fat32 1MiB 45MiB
	dd if=$< of=$@ bs=1M oseek=1 count=$(esp-size) conv=notrunc

esp.img: bootx64.efi kernel.bin
	@rm -fr $@
	mkesp $@ --size $(esp-size)
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

.PHONY: default image vdi clean distclean
.PHONY: attach
