PLUGINS=bitcrusher eq4bp
CLEAN_PLUGINS=$(foreach plugin,$(PLUGINS),clean_$(plugin))

.PHONY: $(PLUGINS) $(CLEAN_PLUGINS)

all: $(PLUGINS)

$(PLUGINS):
	@echo == Building $@ ==
	@cd $@.lv2 && $(MAKE) -s

clean: $(CLEAN_PLUGINS)

$(CLEAN_PLUGINS):
	@echo == Cleaning $(@:clean_%=%) ==
	@cd $(@:clean_%=%).lv2 && $(MAKE) -s clean
