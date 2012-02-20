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
PROJECT		?= bitkeys
LIBPATH     := ../lib/additup ../lib/general
INCLUDE     := ../lib
LDFLAGS     :=
CXXFLAGS    = -Wall -g -DDEBUG -fmessage-length=0

ifneq ($(PLATFORM),win32)
	CXXFLAGS += -ggdb3
	LDFLAGS  += -rdynamic
else
#	CXXFLAGS += -mno-cygwin -mthreads
#	CXXFLAGS += -pg #Generate profiling info.
	CXXFLAGS += -mconsole -mno-cygwin -mthreads -gstabs
	LDFLAGS += -mconsole -mno-cygwin -gstabs
	LIBS += gdi32
endif
LIBS += additup general pthread dl ssl crypto

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
-include ../lib/libadditup.mak

# Auto generated dependencies
include depends.mak

info:
	@echo "--- INFO (make)"
	@echo "PLATFORM   = $(PLATFORM)"
	@echo "PROJECT    = $(PROJECT)$(VERSIONSTRING)"
	@echo "OBJS       = $(OBJS)"
	@echo "CXXFLAGS   = $(CXXFLAGS)"
	@echo "--- INFO (end)"

build: prebuild $(PROJECT)

run: build
	./$(PROJECT) --address 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa
	./$(PROJECT) --address --hex 0062e907b15cbf27d5425399ebf6f0fb50ebb88f18c29b7d93
	./$(PROJECT) --secret 915VSQfWNUB8CihzKjH8FZDChf9ZTdKhS9L5wKqgbJmw
	./$(PROJECT) --secret --hex 76e292975f477698382b1aab142dca51d5cf798ce823111780d98e46a3f28fd2
	./$(PROJECT) --secret generate
	./$(PROJECT) --brainwallet "Man made it to the moon,, and decided it stinked like yellow cheeeese."


# --- Build recipes

$(PROJECT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(patsubst %,-L%,$(LIBPATH)) \
		$(OBJS) $(patsubst %,-l%,$(LIBS))

depends.mak: $(SOURCES) $(SOURCES:.cc=.h)
	@$(CXX) -M $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		$(SOURCES) > $@

%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) \
		$(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		-c $*.cc

# --- Utility recipes
prebuild: tags

tags: $(SOURCES) $(HEADERS)
	ctags --language-force=c++ --extra=+q --fields=+i $(SOURCES) $(HEADERS)

clean:
	-$(RM) tags
	-$(RM) depends.mak
	-$(RM) $(OBJS)
	-$(RM) $(PROJECT)


.PHONY: default info build tests prebuild clean
