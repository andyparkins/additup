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

-include ../platform.mak
-include ../config.mak

SOURCES		:= $(wildcard *.cc)
SOURCES		:= $(filter-out $(REMOVEDSOURCES),$(SOURCES))
HEADERS		:= $(SOURCES:.cc=.h)
OBJS		:= $(SOURCES:.cc=.o)
PROJECT		?= $(notdir $(shell readlink -f $(PWD)/../..))
LIBNAME     := lib$(PROJECT)
LIBPATH     := .
INCLUDE     := .
LDFLAGS     :=
CXXFLAGS    = -Wall -g -DDEBUG -fmessage-length=0

ifneq ($(PLATFORM),win32)
	LIBS     += z \
			    pthread \
			    dl
	CXXFLAGS += -rdynamic -ggdb3
	LDFLAGS  += -rdynamic
else
#	CXXFLAGS += -mno-cygwin -mthreads
#	CXXFLAGS += -pg #Generate profiling info.
	CXXFLAGS += -mconsole -mno-cygwin -mthreads -gstabs
	LDFLAGS += -mconsole -mno-cygwin -gstabs
	LIBS += gdi32 ws2_32 iphlpapi pthreadGC2
endif
LIBS += $(PROJECT)

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

# Per-module Makefile components
-include $(OBJS:.o=.mak)

default: build

info:
	@echo "--- INFO (make)"
	@echo "PLATFORM   = $(PLATFORM)"
	@echo "PROJECT    = $(PROJECT)$(VERSIONSTRING)"
	@echo "CXXFLAGS   = $(CXXFLAGS)"
	@echo "--- INFO (end)"

build: prebuild $(LIBNAME).mak $(LIBNAME).a

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


# ------- tests
-include ../unittest.mak


# --- Build recipes
%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		-c $*.cc

# --- Automatic version module generation
# autoversion.o always wants to have the current output of git-describe;
# however we also don't want to generate an unnecessary compilation (and hence
# link) step if git-describe hasn't changed.  Therefore we don't want to force
# the build of autoversion.o, instead we want to make it dependent on the
# output of git-describe having changed.
#
# Unfortunately make only triggers rebuilds based on file timestamps, so we
# need a method of converting a change of git-describe output into a change of
# a file.  We do this as follows:
#  - The prebuild recipe always runs the version-check recipe
#  - The version-check recipe updates the file version-trigger; but only if the
#    current version is different from the version-built.  This is done by
#    outputting the current version to a temporary file and comparing that file
#    with version-built; if they are different then the version-trigger file
#    has its timestamp updated
#  - The autoversion.o recipe is made dependent on the file version-trigger,
#    and so builds whenever version-trigger is later than autoversion.o
#  - The autoversion.o recipe writes the current version to the file
#    version-built whenever it creates a new autoversion.o ready for the next
#    run of version-check to use as a comparison.
#
# version-trigger exists by the time autoversion.o runs because version-check
# must have already run.
#
# autoversion.o is not dependent on version-check, so make is not forced to
# rebuild it every time.
#
# Finally: if version-trigger is wanted and isn't present, then a simple
# dependency on version-check creates it
#
autoversion.o: autoversion.cc autoversion.h version-trigger
	$(CXX) $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		-c autoversion.cc
	$(STRIP) --strip-debug -R .comment autoversion.o
	@echo $(VERSIONSTRING) > version-built

version-check:
	@echo $(VERSIONSTRING) > thisversion.tmp
	-@cmp -s thisversion.tmp version-built || (echo "Version trigger $(VERSIONSTRING)"; touch version-trigger)
	-@$(RM) thisversion.tmp

version-trigger: version-check

# --- Utility recipes
prebuild: tags version-check

tags: $(SOURCES) $(HEADERS)
	ctags --language-force=c++ --extra=+q --fields=+i $(SOURCES) $(HEADERS)

clean:
	-$(RM) tags
	-$(RM) $(LIBNAME).a $(LIBNAME).mak
	-$(RM) $(OBJS)
	-$(RM) $(UNITEXES)
	-$(RM) $(patsubst %,%.out,$(UNITEXES))
	-$(RM) version-trigger version-built


.PHONY: default info build tests prebuild clean version-check
