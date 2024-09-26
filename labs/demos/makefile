CXX=g++
CXXFLAGS=-std=c++17 -pedantic -Wall -Wextra

SRCS=server.cpp client.cpp
BINS=$(patsubst %.cpp,%.exe,$(SRCS))
DEPS=common.o

all: clean $(BINS)

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.exe: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -o $(patsubst %.exe,%,$@) $^

.PHONY: clean

clean:
	rm -f server client fifo*
