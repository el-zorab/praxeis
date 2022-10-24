ISO_IMAGE := praxeis.iso
QEMU := qemu-system-x86_64
QEMU_FLAGS := -monitor stdio -smp 4 -M q35 -m 2G

.PHONY: all
all: $(ISO_IMAGE)

.PHONY: run
run: $(ISO_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -cdrom $(ISO_IMAGE)

.PHONY: run-kvm
run-kvm: $(ISO_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -enable-kvm -cdrom $(ISO_IMAGE)

.PHONY: debug-qemu
debug-qemu: $(ISO_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -D qemu-log.txt -d int,guest_errors -M smm=off -no-reboot -no-shutdown -cdrom $(ISO_IMAGE)

.PHONY: debug-gdb
debug-gdb: $(ISO_IMAGE)
	$(QEMU) $(QEMU_FLAGS) -s -S -cdrom $(ISO_IMAGE)

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
	$(MAKE) -C src limine
	$(MAKE) -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C src

$(ISO_IMAGE): limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp src/kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO_IMAGE)
	limine/limine-deploy $(ISO_IMAGE)
	# rm -rf iso_root

.PHONY: clean
clean:
	rm -f $(ISO_IMAGE)
	$(MAKE) -C src clean

.PHONY: distclean
distclean:
	rm -rf limine
	$(MAKE) -C src distclean
