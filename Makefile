include Make.common
all: sol test
clean:
	@$(MAKE) -C $(SRCROOT)/sol $@
	@$(MAKE) -C $(SRCROOT)/test $@

sol:
	@$(MAKE) -C $(SRCROOT)/sol $@

test:
	@$(MAKE) -C $(SRCROOT)/test $@

.PHONY: clean sol test
