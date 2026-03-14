CC ?= cc
CFLAGS ?= -g -Wall -O2 -fno-builtin-malloc
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2 -fno-builtin-malloc

SRC = src/allocator.cpp

all: libmymalloc.so

# C++ example:
libmymalloc.so: $(SRC)
	$(CXX) $(CXXFLAGS) -shared -fPIC -ldl -o $@ $<

check: all
	$(MAKE) -C tests check

clean:
	rm -rf *.so *.so.* *.o