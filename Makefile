include Make.common
all: test

clean:
	@$(MAKE) -C $(SRCROOT)/sol $@
	@$(MAKE) -C $(SRCROOT)/test $@
	@$(MAKE) -C $(SRCROOT)/deps/libev $@
	@rm -rf "$(BUILD_PREFIX)/libev"

sol: libev
	@$(MAKE) -C $(SRCROOT)/sol $@

libev:
	@$(MAKE) -C $(SRCROOT)/deps/libev
	@mkdir -p "$(LIB_BUILD_PREFIX)"
	@ln -fs "$(SRCROOT)/deps/libev/.libs/libev.a" "$(LIB_BUILD_PREFIX)"

test:
	@$(MAKE) -C $(SRCROOT)/test $@

.PHONY: clean sol test
