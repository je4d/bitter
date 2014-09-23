subdirs := bitter
includes := external
include modules/jake/jake.mf
#CC=clang++
CXXFLAGS = $(CXXFLAGS_BASE) -fsanitize=address
all: test
