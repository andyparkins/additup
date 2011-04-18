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

-include platform.mak
-include config.mak

# --- Parameters
SUBDIRS		:= lib

# --- Recipes
default: build

info:
	@echo "--- INFO (make)"
	@echo "PLATFORM   = $(PLATFORM)"
	@echo "--- INFO (end)"

build: $(SUBDIRS)
	for d in $(SUBDIRS); do $(MAKE) -C $$d; done

clean:
	for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done


.PHONY: default info build
