#!/bin/bash

SCRIPT_EXEC=$(cat << EOF

export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1

mkdir -p /home/duckdb/fuzzing/fuzz_root/outputs/
mkdir -p /home/duckdb/fuzzing/fuzz_root/outputs/outputs_$1/
cd /home/duckdb/fuzzing/fuzz_root/outputs/outputs_$1/

cp /home/duckdb/fuzzing/fuzz_root/parserfuzz ./parserfuzz
cp /home/duckdb/fuzzing/fuzz_root/duckdb ./
cp /home/duckdb/fuzzing/fuzz_root/duckdb_grammar_modi.y ./
cp /home/duckdb/fuzzing/fuzz_root/ir_types_mapping.txt ./
cp /home/duckdb/fuzzing/fuzz_root/function_type_lib.json ./
cp /home/duckdb/fuzzing/fuzz_root/set_session_variables.json ./
cp -r /home/duckdb/fuzzing/fuzz_root/inputs ./inputs

gdb -ex=r --args ./parserfuzz -i ./inputs -o ./ -c $1 -t 2000 -m none --  ./duckdb

EOF
)

echo ""
echo "Begin Fuzzing with core $1"
su -c "$SCRIPT_EXEC" duckdb
echo "Finished"
echo ""



