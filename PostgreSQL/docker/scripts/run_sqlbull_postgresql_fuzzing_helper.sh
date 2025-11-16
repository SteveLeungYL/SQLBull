#!/bin/bash -e

# This file is used for start the SQLRight MySQL fuzzing inside the Docker env.
# entrypoint: bash

SCRIPT_EXEC=$(cat << EOF
# Setup data folder
printf "\n\n\n\nStart fuzzing. \n\n\n\n\n"

cd /home/postgresql/fuzzing/fuzz_root

python3 run_parallel.py -c $1

while true; do foo; sleep 2; done

EOF
)

su -c "$SCRIPT_EXEC" postgresql

echo "Finished\n"