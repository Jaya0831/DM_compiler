subprojects := compute memory
clean_subprojects := $(addprefix clean-,$(subprojects))

export CFLAGS

.PHONY: $(subprojects) clean $(clean_subprojects) .FORCE
.FORCE:

all: $(subprojects)

$(subprojects): %:
	$(MAKE) -C $@

clean: $(clean_subprojects)

$(clean_subprojects): %:
	$(MAKE) -C $(patsubst clean-%,%,$@) clean

compile_commands.json: .FORCE
	$(MAKE) clean
	bear -- $(MAKE)
