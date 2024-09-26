#!/usr/bin/env bash

# function to clean up files and make executables
remake () {
    make -s clean
    make -s >/dev/null 2>&1
}

ORANGE='\033[0;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

PTS1=1
PTS2=1
PTS3=1

echo -e "\nTesting :: Compilation\n"
make -s clean
if make -s; then
    echo -e "  ${GREEN}Passed${NC}"
else
    echo -e "  ${RED}Failed${NC}"
fi

test() {
    echo -e "\nTesting :: ./problemA test-files/test-$1-in.txt $2\n"

    timeout 1 ./problemA test-files/test-$1-in.txt $2 >test-files/out.txt 2>test-files/err.txt

    #test that the right number of threads is created
    printf "  Checking # of threads:        "
    if [ $(awk '{print $1}' test-files/err.txt | uniq | wc -l) -eq $2 ]; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS1=0
    fi

    #test that each thread sums the right amount of numbers
    printf "  Checking per-thread sum size: "
    if [ $(awk '{print $2}' test-files/err.txt | uniq) == $3 ]; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS2=0
    fi

    #test the output
    printf "  Checking output:              "
    if diff -q test-files/out.txt test-files/test-$1-out.txt &>/dev/null; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS3=0
    fi
    
    # echo -e "  ${GREEN}Passed${NC}"

}

test 1 4 3
test 2 10 50
test 3 100 4

PTS=$(($PTS1+$PTS2+$PTS3))

echo -e "\n\n"
echo "Points:" $PTS/3
echo -e "\n"

exit 0