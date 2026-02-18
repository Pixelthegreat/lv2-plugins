include common.mk

PLUGINS=bitcrusher delay eq4bp mixer-strip
LIBRARIES=ui biquad

CLEAN_PLUGINS=$(foreach plugin,$(PLUGINS),clean_$(plugin))
CLEAN_LIBRARIES=$(foreach library,$(LIBRARIES),clean_$(library))

.PHONY: clean
.PHONY: $(LIBRARIES) $(CLEAN_LIBRARIES)
.PHONY: $(PLUGINS) $(CLEAN_PLUGINS)

all: $(LIBRARIES) $(PLUGINS)

$(LIBRARIES):
	@echo == Building $@ ==
	@cd $@ && $(MAKE) -s

$(PLUGINS):
	@echo == Building $@ ==
	@cd $@.lv2 && $(MAKE) -s

clean: $(CLEAN_LIBRARIES) $(CLEAN_PLUGINS)

$(CLEAN_LIBRARIES):
	@echo == Cleaning $(@:clean_%=%) ==
	@cd $(@:clean_%=%) && $(MAKE) -s clean

$(CLEAN_PLUGINS):
	@echo == Cleaning $(@:clean_%=%) ==
	@cd $(@:clean_%=%).lv2 && $(MAKE) -s clean
