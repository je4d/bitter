subdirs := bitter
includes := external
include modules/jake/jake.mf
#CC=clang++
CXXFLAGS_BASE = -g -std=c++14 -Wall -Werror -pedantic -Iinclude -ftemplate-backtrace-limit=0 -fdiagnostics-color=$(GCC_COLOR)
CXXFLAGS = $(CXXFLAGS_BASE) -fsanitize=address -O
all: test
