#!/bin/bash
cmake --build build
if [[ ! $? -eq 0  ]]
then
    echo "Failed to build project"
    exit 1
fi

if [[ ! -d build ]]
then
    cmake -B build
    if [[ $? -eq 0 ]]; then
        echo "Failed to create CMake build directory"
        exit 2
    fi
fi

if [[ ! -d build/Tests ]]
then
    echo "No tests to run"
    exit
fi

echo -e "\n+-----------------------+"
echo      "|     RUNNING TESTS     |"
echo -e   "+-----------------------+\n"
testfiles=$(ls ./build/Tests/ | grep -E ^.*Tests$)
for testfile in $testfiles
do
    echo "--- Running $testfile ---"
    ./build/Tests/$testfile
done
