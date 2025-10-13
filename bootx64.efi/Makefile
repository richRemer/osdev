bin = bootx64.efi
cc = clang
link = lld-link
cc-opts = -target x86_64-pc-win32-coff -fno-stack-protector -fshort-wchar -mno-red-zone -I/usr/include/efi
link-opts = -filealign:16 -nodefaultlib -entry:efi_main -subsystem:efi_application

default: build

build: $(bin)

clean:
	rm -fr *.efi *.o

%.o: %.c
	$(cc) $(cc-opts) -c $< -o $@

$(bin): $(patsubst %.c,%.o,$(wildcard *.c))
	$(link) $(link-opts) $^ -out:$@
