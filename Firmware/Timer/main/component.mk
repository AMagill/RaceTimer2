#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)




COMPONENT_PRIV_INCLUDEDIRS += ../VisualGDBCache/Timer-Debug/CodeSenseDir/include
CXXFLAGS += $(COMPONENT_PRIV_COMMONFLAGS)
CFLAGS += $(COMPONENT_PRIV_COMMONFLAGS)
COMPONENT_PRIV_COMMONFLAGS += 