name := compute

include_dir := include
headers := $(wildcard src/*.h) $(wildcard $(include_dir)/*.h) $(wildcard ../common/*.h)

sources := $(wildcard src/*.c)
objects := $(sources:.c=.o)

tests_sources := $(wildcard tests/*.c)
tests_objects := $(tests_sources:.c=.o)

mkdir_out := mkdir -p out

CFLAGS += -fPIC

.PHONY: build build-shared build-static build-tests clean

all: build
build: build-shared build-static build-tests
build-shared: out/lib$(name).so
build-static: out/lib$(name).a
build-tests: out/run-test

clean:
	find . -type f -name *.o -delete
	rm -rf out

$(objects) $(tests_objects): %.o: %.c $(headers)
	$(CC) $(CFLAGS) -c -o $@ $< -I$(include_dir)

out/lib$(name).so: $(objects)
	$(mkdir_out)
	$(CC) $(LDFLAGS) -shared -o $@ $^

out/lib$(name).a: $(objects)
	$(mkdir_out)
	$(AR) rcs $@ $^

out/run-test: $(tests_objects) out/lib$(name).a
	$(CC) $(LDFLAGS) -o $@ $^ -libverbs -lrdmacm
