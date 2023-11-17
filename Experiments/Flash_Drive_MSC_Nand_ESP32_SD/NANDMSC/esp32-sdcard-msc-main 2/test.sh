#!/bin/bash

# Number of tests to run
num_tests=5

# Size of the file to be written/read (in MB)
filesize=1024  # 1GB

blocksize=100K # 100MB in total

# File to write/read
testfile="tempfile"

# Arrays to store results
write_results=()
read_results=()

# 1. Write Test Loop
for ((i=1; i<=num_tests; i++))
do
    echo "Write test $i..."
    result=$(dd if=/dev/zero of=$testfile$i bs=$blocksize count=$filesize conv=notrunc 2>&1 | awk -F'[()]' '{print $2}' | awk '{print $1}')
    speed=${result% B/s}
    write_results+=($speed)
    echo "Write Speed: $speed B/s"
done

# 2. Read Test Loop
for ((i=1; i<=num_tests; i++))
do
    echo "Read test $i..."
    sudo purge
    result=$(dd if=$testfile$i of=/dev/null bs=$blocksize count=$filesize 2>&1 | awk -F'[()]' '{print $2}' | awk '{print $1}')
    speed=${result% B/s}
    read_results+=($speed)
    echo "Read Speed: $speed B/s"
done

# 3. Cleanup
rm $testfile?

# 4. Analyze Results (Calculate Average and Standard Deviation)
# You might want to do this analysis part using a tool like Python for simplicity and accuracy.

echo "Write Speeds: ${write_results[@]} B/s"
echo "Read Speeds: ${read_results[@]} B/s"

echo ${write_results[@]} > write_results.txt
echo ${read_results[@]} > read_results.txt

# Optionally, you can call a Python script here to analyze the results
# echo ${write_results[@]} | python3 your_analysis_script.py
# echo ${read_results[@]} | python3 your_analysis_script.py

