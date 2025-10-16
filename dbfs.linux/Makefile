bin = dbfs.linux

default: build

build: $(bin)

clean:
	rm -rf $(bin) *.o

$(bin): $(patsubst %.c,%.o,$(wildcard *.c))
	clang -lfuse3 $^ -o $@

%.o: %.c
	clang -c $< -o $@

.PHONY: default build clean
