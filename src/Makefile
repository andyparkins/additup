# ----------------------------------------------------------------------------
# Version Control
#    $Author$
#      $Date$
#        $Id$
#
# Legal
#    Copyright 2011  Andy Parkins
#
# ----------------------------------------------------------------------------

# --- Parameters
DEPS := lib bitkeys

# --- Recipes
default: build

build: $(DEPS:=.d)

clean: $(DEPS:=.d.clean)

%.d: %
	$(MAKE) -C $*

%.d.clean: %
	$(MAKE) -C $* clean

#.PHONY: default build clean $(DEPS:=.d) $(DEPS:=.d.clean)
