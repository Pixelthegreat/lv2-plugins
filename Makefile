PLUGINS=bitcrusher delay eq4bp
CLEAN_PLUGINS=$(foreach plugin,$(PLUGINS),clean_$(plugin))

.PHONY: ui clean_ui $(PLUGINS) $(CLEAN_PLUGINS)

all: ui $(PLUGINS)

$(PLUGINS):
	@echo == Building $@ ==
	@cd $@.lv2 && $(MAKE) -s

ui:
	@echo == Building ui ==
	@cd ui && $(MAKE) -s

clean: clean_ui $(CLEAN_PLUGINS)

clean_ui:
	@echo == Cleaning ui ==
	@cd ui && $(MAKE) -s clean

$(CLEAN_PLUGINS):
	@echo == Cleaning $(@:clean_%=%) ==
	@cd $(@:clean_%=%).lv2 && $(MAKE) -s clean
