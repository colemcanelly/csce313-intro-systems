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

test () {
    echo -e "\nTesting :: ./problemB $2 <test-files/test-$1-in.txt\n"

    timeout 1 ./problemB <test-files/test-$1-in.txt $2 >test-files/out.txt 2>test-files/err.txt

    #test that the right number of threads is created
    printf "  Checking # of threads:        "
    if [ $(awk '{print $1}' test-files/out.txt | sort | uniq | wc -l) -eq $2 ]; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS1=0
    fi

    #test the output (should be same as input)
    printf "  Checking output:              "
    awk '{print $2}' test-files/out.txt > test-files/tmp.txt
    if diff -q test-files/tmp.txt test-files/test-$1-in.txt &>/dev/null; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS2=0
    fi

    #test that the consumers start and exit successfully
    printf "  Checking consumer trace:      "
    if diff -q test-files/err.txt test-files/test-$1-err.txt &>/dev/null; then
        echo -e "  ${GREEN}Passed${NC}"
    else
        echo -e "  ${RED}Failed${NC}"
        PTS3=0
    fi
    
    # echo -e "  ${GREEN}Passed${NC}"
}

test 1 2
test 2 4
test 3 10

PTS=$(($PTS1+$PTS2+$PTS3))

echo -e "\n\n"
echo "Points:" $PTS/3
echo -e "\n"

exit 0