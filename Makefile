# Default optimization level
OPT ?= -O0

# Flags
CXXFLAGS := -std=gnu++1z -W -Wall -Wshadow -Wno-implicit-fallthrough -g $(OPT) $(DEFS) $(CXXFLAGS)
LDFLAGS := -no-pie

# Make sanitizers available as a compilation option ("make SAN=1")
#
# use SAN=1 to turn on all sanitizers (address, leak, undefined behavior)
#
# to turn on specific sanitizers, use:
# - ASAN=1 for the address sanitizer
# - LSAN=1 for the leak sanitizer
# - UBSAN=1 for the undefined behavior sanitizer
-include sanitizers.mk

all: repl

repl: repl.cpp index.hpp index.cpp query.hpp query.cpp text_processor.hpp text_processor.cpp stemmer/porter2_stemmer.hpp stemmer/porter2_stemmer.cpp util/util.hpp util/util.cpp pugixml/pugixml.hpp pugixml/pugixml.cpp
	@echo "Compiling repl.cpp..."
	g++ $(CXXFLAGS) -pthread repl.cpp index.cpp query.cpp text_processor.cpp stemmer/porter2_stemmer.cpp util/util.cpp pugixml/pugixml.cpp -o repl
	@echo "Compilation completed."
	clear

clean:
	@echo "Cleaning up..."
	@rm -f repl
	@echo "Cleanup completed."
	clear