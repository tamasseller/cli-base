curdir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

INCLUDE_DIRS := $(INCLUDE_DIRS) $(curdir)/.

SOURCES := $(SOURCES) $(curdir)/CliApp.cpp
SOURCES := $(SOURCES) $(curdir)/Levenshtein.cpp
SOURCES := $(SOURCES) $(curdir)/OptionParser.cpp
SOURCES := $(SOURCES) $(curdir)/Autocomplete.cpp


undefine curdir