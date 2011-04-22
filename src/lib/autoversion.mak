-include ../revision.mak
autoversion_DEFINES := VCSID=\"$(VERSIONSTRING)\"

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

