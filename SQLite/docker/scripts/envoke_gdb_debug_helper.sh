#!/bin/bash

SCRIPT_EXEC=$(cat << EOF
mkdir -p /home/sqlite/fuzzing/fuzz_root/outputs
mkdir -p /home/sqlite/fuzzing/fuzz_root/outputs/outputs_$1/
cd /home/sqlite/fuzzing/fuzz_root/

gdb -ex=r --args ./parserfuzz -m none -t 2000 -i ./inputs -o /home/sqlite/fuzzing/fuzz_root/outputs/outputs_$1 -c $1 --  /home/fuzzing/fuzz_root/sqlite3

EOF
)

echo ""
echo "Begin Fuzzing with core $1"
su -c "$SCRIPT_EXEC" sqlite
echo "Finished"
echo ""
