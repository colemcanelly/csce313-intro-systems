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

PTS=0

echo -e "\nTesting :: Compilation\n"
make -s clean
if make -s; then
    echo -e "  ${GREEN}Passed${NC}"
else
    echo -e "  ${RED}Failed${NC}"
fi

outtest () {
    echo -e "\nTesting :: Output ( ./problemC $2 $3 )\n"
    timeout 1 ./problemC $2 $3 >test-files/out.txt 2>test-files/err.txt
    if [ $(uniq < test-files/err.txt | wc -l) -eq $2 ]; then 
        if diff test-files/test-$1.txt test-files/out.txt &>/dev/null; then
            echo -e "  ${GREEN}Passed${NC}"
            PTS=$((PTS+1))
        else
            echo -e "  ${RED}Failed${NC}"
        fi
    else
        echo -e "  ${RED}Failed${NC}"
    fi
}

outtest 1 3 1
outtest 2 3 2
outtest 3 5 5
outtest 4 25 40

echo -e "\n\n"
echo "Points:" $PTS/4
echo -e "\n"

exit 0