#!/usr/bin/env bash

remake () {
    #echo -e "\nCleaning old files and making executables"
    make -s clean
    #echo -e "\nRedirecting stdout & stderr to background so it doesnt print to screen
    make -s >/dev/null 2>&1
}

# Define colors to pretty outputs
ORANGE='\033[0;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# function to check for IPC files (what does this do)
checkclean () {
    if [ "$1" = "f" ]; then
        if [ "$(find . -type p)" ]; then
            #echo "Failed to close FIFORequestChannel - removing for next test"
            find . -type p -exec rm {} +
        fi
    else
        echo -e "  ${RED}something broke lol${NC}"
        exit 1
    fi
}

OUTPUT='report/times.csv'
FILE='test.bin'
REQUEST='./client'
TRUNC='truncate'


echo "Buffer (bytes),Filesize (bytes),Time (s)" > "${OUTPUT}"

# function to time the execution of the client process
time_exec () {
    if [ $# -eq 0 ]; then
        echo  -e "  ${RED}No arguments supplied${NC}"
        return
    fi
    local size="$1"
    local buffer="256"

    remake
    TRUNC="${TRUNC} -s ${size} BIMDC/${FILE}"   # truncate -s <size> BIMDC/test.bin
    ${TRUNC}
    # echo -e "Ran: ${TRUNC}"

    REQUEST="${REQUEST} -f ${FILE}"             # ./client -f <filename>

    echo -e "${GREEN}\nTiming :: ${REQUEST}${NC}"
    if [ -z "$2" ]; then
        echo -e "\tDefault Buffer (256)"
    else
        buffer="$2"
        echo -e "\tBuffer = ${buffer}"
        REQUEST="${REQUEST} -m ${buffer}"       # ./client -f <filename> -m <buffer_capacity>
    fi
    echo -e "\tSize = ${size}"
    
    start=`date +%s.%N`
    "${REQUEST}" >/dev/null 2>&1
    end=`date +%s.%N`
    runtime=$( echo "$end - $start" | bc -l)
    # echo -e "Ran: ${REQUEST}"
    
    echo -e "\tTime = ${runtime} seconds\n"

    echo "${buffer},${size},${runtime}" >> "${OUTPUT}"
    checkclean "f"
}

time_exec "256K"
time_exec "512K"
time_exec "768K"
time_exec "1024K" #1MB
time_exec "1280K"
time_exec "1536K"
time_exec "1792K"
time_exec "2048K" #2MB
time_exec "10240K" #10MB
time_exec "102400K" #100MB


time_exec "100M"
time_exec "100M" "512"
time_exec "100M" "1024"
time_exec "100M" "1536"
time_exec "100M" "2048"
time_exec "100M" "2560"



