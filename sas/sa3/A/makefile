CXX=g++
CXXFLAGS=-std=c++17 -g -pedantic -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer

all: problemA

.PHONY: clean test

clean:
	rm -f problemA test-files/out.txt test-files/err.txt

test:
	chmod u+x SA3A-tests.sh
	./SA3A-tests.sh