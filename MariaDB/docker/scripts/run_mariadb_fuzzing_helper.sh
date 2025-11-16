#!/bin/bash -e

# This file is used for start the SQLBull MariaDB fuzzing inside the Docker env.
# entrypoint: bash

SCRIPT_EXEC=$(cat << EOF
# Setup data folder
printf "\n\n\n\nStart fuzzing. \n\n\n\n\n"

cd /home/mariadb/fuzzing/fuzz_root

python3 run_parallel.py -c $1

sleep 1000000

EOF
)

su -c "$SCRIPT_EXEC" mariadb

echo "Finished\n"