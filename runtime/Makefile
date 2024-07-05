subprojects := compute memory
clean_subprojects := $(addprefix clean-,$(subprojects))

export CFLAGS

.PHONY: $(subprojects) clean $(clean_subprojects)

all: $(subprojects)

$(subprojects): %:
	$(MAKE) -C $@

clean: $(clean_subprojects)

$(clean_subprojects): %:
	$(MAKE) -C $(patsubst clean-%,%,$@) clean
