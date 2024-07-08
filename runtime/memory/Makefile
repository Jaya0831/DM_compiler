name := memory-server

sources := $(wildcard src/*.c)
objects := $(sources:.c=.o)
headers := $(wildcard src/*.h) $(wildcard ../common/*.h)

deps := ibverbs rdmacm
deps_flags := $(patsubst %,-l%,$(deps))

mkdir_out := mkdir -p out

.PHONY: build

all: build
build: out/$(name)

clean:
	find . -type f -name *.o -delete
	rm -rf out

$(objects): %.o: %.c $(headers)
	$(CC) $(CFLAGS) -c -o $@ $<

out/$(name): $(objects)
	$(mkdir_out)
	$(CC) $(LDFLAGS) -o $@ $^ $(deps_flags)
