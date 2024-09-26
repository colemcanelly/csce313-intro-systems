CXX=g++
CXXFLAGS=-std=c++17 -g -pedantic -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer

all: problemC

.PHONY: clean test

clean: 
	rm -f problemC test-files/out.txt test-files/err.txt

test:
	chmod u+x SA3C-tests.sh
	./SA3C-tests.sh
