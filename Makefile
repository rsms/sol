include Make.common
all: sol test
clean:
	@$(MAKE) -C $(SRCROOT)/sol $@
	@$(MAKE) -C $(SRCROOT)/test $@
	@$(MAKE) -C $(SRCROOT)/deps/libev $@
	@rm -rf "$(BUILD_PREFIX)/libev"

sol: libev
	@$(MAKE) -C $(SRCROOT)/sol $@

libev:
	@$(MAKE) -C $(SRCROOT)/deps/libev
	@mkdir -p "$(BUILD_PREFIX)/libev"
	ln -fs "$(SRCROOT)/deps/libev/.libs/libev.a" "$(BUILD_PREFIX)/libev"

test:
	@$(MAKE) -C $(SRCROOT)/test $@

.PHONY: clean sol test
