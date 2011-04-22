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

# Testable modules are any that have 'ifdef UNITTEST' in them"
TESTABLE := $(shell grep -ls "ifdef UNITTEST" $(SOURCES))

# Expand to make convenience targets for the user to "make"
UNITEXES := $(patsubst %.cc,unit-%,$(TESTABLE))
RUNTESTS := $(patsubst %.cc,test-%,$(TESTABLE))

unittestinfo:
	@echo "TESTABLE = $(TESTABLE)"
	@echo "UNITEXES = $(UNITEXES)"
	@echo "RUNTESTS = $(RUNTESTS)"

# Run all the tests
tests: $(UNITEXES) $(RUNTESTS)
	@echo "*** SUCCESS *** $(TESTABLE)"

# valgrind a particular unit
grind-%: unit-%
	-rm -f vg-$*.log
	-rm -f vg-$*.log.core*
	valgrind --track-fds=yes --show-reachable=yes --leak-check=yes \
		--log-file=vg-$*.log \
		./unit-$* > unit-$*.out

# run a particular unit
test-%: unit-%
	-@rm -f core*
	@echo "[$*] --------------------------- Running module"
	@./unit-$* > unit-$*.out || echo "[$*] Error $$? in module unit test"
	@echo "[$*] --------------------------- Success running module $*"

# compile a particular unit.  This recipe automatically adds in per-module
# includes, libarary paths, include paths, libraries and defines.  UNITTEST
# is automatically defined, which the module can wrap its UNITTEST main() in.
# LDFLAGS and CXXFLAGS are used as normal.
unit-%: %.cc $(LIBNAME).a
	$(CXX) $*.cc $(CXXFLAGS) -O0 \
		-DUNITTEST $(patsubst %,"-D%",$($*_DEFINES)) \
		$(patsubst %,-I%,$(INCLUDE)) $(patsubst %,-I%,$($*_INCLUDE)) \
		-o unit-$* $(LDFLAGS) \
		$(patsubst %,-L%,$(LIBPATH)) $(patsubst %,-L%,$($*_LIBPATH)) \
		$(patsubst %,-l%,$(LIBS)) $(patsubst %,-l%,$($*_LIBS))
