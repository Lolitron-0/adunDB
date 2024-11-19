#!/bin/bash
# Convenience script for local builds - feel free to change it to your liking

set -eu

mkdir -p build 

HI='\033[0;32m'
NC='\033[0m'

cmake -S . \
			-B build \
			-G Ninja \
			-DCMAKE_CXX_COMPILER=clang++ \
			-DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -pedantic -fprofile-instr-generate -fcoverage-mapping" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=1 
cmake --build build --parallel $(nproc)
cp -f build/compile_commands.json .

if [ $# -ne 0 ] ; then
	echo -e "$HI--------------------------------------$NC\n"
  if [ "$1" == "run" ]; then
    set -eux
    ./build/tests/adundb_tests_sandbox 
  elif [ "$1" == "test" ]; then
    pushd build/tests > /dev/null
    rm -rf coverage
    mkdir -p coverage/report
    test_name=adundb_tests_unit
    LLVM_PROFILE_FILE="coverage/%p.profraw" ./$test_name
    llvm-profdata merge -sparse coverage/*.profraw -o coverage/$test_name.profdata
    llvm-cov show ./$test_name -instr-profile=coverage/$test_name.profdata -format=html -output-dir=coverage/report --ignore-filename-regex=build/.*
    popd > /dev/null
  fi
fi
