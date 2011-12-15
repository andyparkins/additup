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

-include ../../platform.mak
-include ../../config.mak

SOURCES		:= $(wildcard *.cc)
SOURCES		:= $(filter-out $(REMOVEDSOURCES),$(SOURCES))
HEADERS		:= $(SOURCES:.cc=.h)
OBJS		:= $(SOURCES:.cc=.o)
PROJECT		?= general
LIBNAME     := lib$(PROJECT)
INCLUDE     := .
LIBPATH     := .
LDFLAGS     :=
CXXFLAGS    = -Wall -g -DDEBUG -fmessage-length=0

ifneq ($(PLATFORM),win32)
	CXXFLAGS += -rdynamic -ggdb3
	LDFLAGS  += -rdynamic
else
#	CXXFLAGS += -mno-cygwin -mthreads
#	CXXFLAGS += -pg #Generate profiling info.
	CXXFLAGS += -mconsole -mno-cygwin -mthreads -gstabs
	LDFLAGS += -mconsole -mno-cygwin -gstabs
	LIBS += gdi32
endif
LIBS += $(PROJECT) pthread dl ssl crypto

ifdef strict
	# --- strict compilation
	CXXFLAGS += -pedantic
#	CXXFLAGS += -Werror
	CXXFLAGS += -Wpointer-arith
	CXXFLAGS += -Wcast-align
	CXXFLAGS += -Wwrite-strings
#	CXXFLAGS += -Wsign-compare
#	CXXFLAGS += -Wfloat-equal
#	CXXFLAGS += -Wextra
#	CXXFLAGS += -Wconversion
	CFLAGS += -std=c99
	CFLAGS += -Wfour-char-constants
	CFLAGS += -Wdeclaration-after-statement
	CFLAGS += -Wbad-function-cast
	CFLAGS += -fnon-lvalue-assign
	CFLAGS += -Wshorten-64-to-32
endif

default: build

# Per-module Makefile components
-include $(OBJS:.o=.mak)

# Auto generated dependencies
include depends.mak

info:
	@echo "--- INFO (make)"
	@echo "PLATFORM   = $(PLATFORM)"
	@echo "PROJECT    = $(PROJECT)$(VERSIONSTRING)"
	@echo "LIBNAME    = $(LIBNAME)"
	@echo "CXXFLAGS   = $(CXXFLAGS)"
	@echo "--- INFO (end)"

build: prebuild $(LIBNAME).mak $(LIBNAME).a


# Unit test standard makefile fragment
UNITLIBS := $(LIBNAME).a
include ../../unittest.mak


# --- Build recipes
$(LIBNAME).a: $(OBJS)
	-$(RM) $@
	$(AR) rcs $@ $(OBJS)

# Pull the dependent library list out of the per-module variables
$(LIBNAME).mak: $(filter-out $(LIBNAME).mak,$(wildcard *.mak))
	@echo "$(LIBNAME)_LIBS := $(sort $(foreach x, \
			$(patsubst %.o,%_LIBS,$(OBJS)), \
			$($x)))" > $@
	@echo "$(LIBNAME)_EXTERNINCLUDE := $(sort $(foreach x, \
			$(patsubst %.o,%_EXTERNINCLUDE,$(OBJS)), \
			$($x)))" >> $@
	@echo "$(LIBNAME)_LIBPATH := $(sort $(foreach x, \
			$(patsubst %.o,%_LIBPATH,$(OBJS)), \
			$($x)))" >> $@

depends.mak: $(SOURCES)
	@$(CXX) -M $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		$^ > $@

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		-c $*.cc

# --- Utility recipes
prebuild: tags version-check

tags: $(SOURCES) $(HEADERS)
	ctags --language-force=c++ --extra=+q --fields=+i $(SOURCES) $(HEADERS)

clean:
	-$(RM) tags
	-$(RM) depends.mak
	-$(RM) $(LIBNAME).a $(LIBNAME).mak
	-$(RM) $(OBJS)
	-$(RM) $(UNITEXES)
	-$(RM) $(patsubst %,%.out,$(UNITEXES))
	-$(RM) version-trigger version-built


.PHONY: default info build tests prebuild clean version-check
