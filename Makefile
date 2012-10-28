include Make.common
all: test sol

clean:
	@$(MAKE) -C $(SRCROOT)/sol $@
	@$(MAKE) -C $(SRCROOT)/test $@

sol:
	@$(MAKE) -C $(SRCROOT)/sol $@

# Shorthand for "make DEBUG=1 sol"
debug:
	@$(MAKE) -C $(SRCROOT)/sol DEBUG=1

test:
	@$(MAKE) -C $(SRCROOT)/test $@

.PHONY: clean sol debug test
