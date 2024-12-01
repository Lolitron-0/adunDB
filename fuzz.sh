#!/bin/bash

export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1

afl-fuzz -i fuzz/in -o fuzz/out build/fuzz/adundb_fuzz_parser
