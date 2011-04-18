# ----------------------------------------------------------------------------
# Version Control
#    $Author: andyp $
#      $Date$
#        $Id$
#
# Legal
#    Copyright 2011  Andy Parkins
#
# ----------------------------------------------------------------------------

VERSIONSTRINGRAW    := $(shell git describe 2> /dev/null )
TAGGEDVERSION       := $(shell git describe --abbrev=0 2> /dev/null)

ifeq ($(VERSIONSTRINGRAW),)
VERSIONSTRINGRAW    := $(shell date -u +%Y%m%d%H)
endif

VERSIONSTRING       := -$(VERSIONSTRINGRAW)
VERSIONSTRINGPREFIX := $(VERSIONSTRINGRAW)-
