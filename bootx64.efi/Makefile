bin = bootx64.efi
sources = build.zig build.zig.zon $(wildcard src/*)

default: build

build: $(bin)

clean:
	rm -fr $(bin) zig-out .zig-cache

$(bin): zig-out/bin/$(bin)
	cp $< $@

zig-out/bin/$(bin): $(sources)
	zig build --verbose

.PHONY: default build clean
