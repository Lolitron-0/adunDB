#!/bin/bash

set -eu

mkdir -p build 

HI='\033[0;32m'
NC='\033[0m'

cmake -S . \
			-B build \
			-G Ninja \
			-DCMAKE_CXX_COMPILER=clang++ \
			-DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -pedantic" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=1 
cmake --build build --parallel $(nproc)
cp -f build/compile_commands.json .

if [ $# -ne 0 ] && [ "$1" == "run" ]; then
	echo -e "----------------------------------\n"
  set -eux
	./build/adundb 
  set +x
  set -eu
fi
