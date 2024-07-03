name := compute

sources := $(wildcard src/*.c)
objects := $(sources:.c=.o)
include_dir := ./include
headers := $(wildcard src/*.h) $(wildcard $(include_dir)/*.h)

mkdir_out := mkdir -p out

.PHONY: build build-shared build-static clean

all: build
build: build-shared build-static
build-shared: out/lib$(name).so
build-static: out/lib$(name).a

clean:
	find . -type f -name *.o -delete
	rm -rf out

$(objects): %.o: %.c $(headers)
	$(CC) -c -o $@ $< -I$(include_dir) $(CFLAGS)

out/lib$(name).so: $(objects)
	$(mkdir_out)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

out/lib$(name).a: $(objects)
	$(mkdir_out)
	$(AR) rcs $@ $^
