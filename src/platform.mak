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

# This makefile fragment automatically sets some variables that help with
# compiling a cross-platform project.

# There are four possibilities to be dealt with:
#   - Linux host,   Compiling for Linux platform
#   - Linux host,   Compiling for Windows platform
#   - Windows host, Compiling for Windows platform
#   - Windows MSYS host, Compiling for Windows platform
# This section sets up the right variables for each case ready for the rest of
# the makefile.
# First, we must detect a Windows host
ifneq ($(WINDIR)$(windir),)
#   In cygwin, the variable $(WINDIR) exists.  From a command line running
#   mingw32-make $(windir) exists.  The presence of either means that we are
#   compiling on windows.
    BUILDHOST           := win32
ifneq ($(MSYSTEM),)
	MAKEMODE            := unix
else
	MAKEMODE            := win32
endif
else
    BUILDHOST           := unix
	MAKEMODE            := unix
endif


# Host-specific settings
ifeq ($(BUILDHOST),win32)
#   The only possible target on Windows is Windows
    PLATFORM       := win32
ifeq ($(MAKEMODE),unix)
	RM             := rm -f
	COPY           := cp
	COPY_DIR       := cp -r
	MSPWD          := $(shell cmd //c echo $(shell pwd) | tr / \\)
else
	RM             := del
	COPY           := copy /y
	COPY_DIR       := xcopy /x /q /y /i
	MSPWD          := $(shell cd)
endif
else
    RM             := rm -f
#   If BUILDHOST isn't win32, then we're compiling on UNIX, in that case, if
#   $PLATFORM is win32 we want to use the mingw32 cross compiler
ifeq ($(PLATFORM),win32)
    BINUTILSPREFIX := i586-mingw32msvc
    CXX            := $(BINUTILSPREFIX)-g++
    CC             := $(BINUTILSPREFIX)-gcc
    CPP            := $(BINUTILSPREFIX)-cpp
    ADDR2LINE      := $(BINUTILSPREFIX)-addr2line
    AR             := $(BINUTILSPREFIX)-ar
    AS             := $(BINUTILSPREFIX)-as
    CXXFILT        := $(BINUTILSPREFIX)-c++filt
    CPP            := $(BINUTILSPREFIX)-cpp
    DLLTOOL        := $(BINUTILSPREFIX)-dlltool
    DLLWRAP        := $(BINUTILSPREFIX)-dllwrap
    GCCBUG         := $(BINUTILSPREFIX)-gccbug
    GCOV           := $(BINUTILSPREFIX)-gcov
    GPROF          := $(BINUTILSPREFIX)-gprof
    LD             := $(BINUTILSPREFIX)-ld
    NM             := $(BINUTILSPREFIX)-nm
    OBJCOPY        := $(BINUTILSPREFIX)-objcopy
    OBJDUMP        := $(BINUTILSPREFIX)-objdump
    RANLIB         := $(BINUTILSPREFIX)-ranlib
    READELF        := $(BINUTILSPREFIX)-readelf
    SIZE           := $(BINUTILSPREFIX)-size
    STRINGS        := $(BINUTILSPREFIX)-strings
    STRIP          := $(BINUTILSPREFIX)-strip
    WINDMC         := $(BINUTILSPREFIX)-windmc
    WINDRES        := $(BINUTILSPREFIX)-windres
else
# Building on UNIX for UNIX
    STRIP          := strip
endif
endif

