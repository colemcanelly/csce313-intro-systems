CXX=g++
CXXFLAGS=-std=c++17 -g -pedantic -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer

all: problemB

.PHONY: clean test

clean:
	rm -f problemB test-files/out.txt test-files/err.txt test-files/tmp.txt

test:
	chmod u+x SA3B-tests.sh
	./SA3B-tests.sh