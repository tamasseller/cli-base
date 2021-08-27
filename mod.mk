curdir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

INCLUDE_DIRS := $(INCLUDE_DIRS) $(curdir)/.

SOURCES := $(SOURCES) $(curdir)/CliApp.cpp
SOURCES := $(SOURCES) $(curdir)/OptionParser.cpp

undefine curdir