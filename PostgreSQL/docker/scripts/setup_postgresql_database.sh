#!/bin/bash -e

# Setup data folder
cd /home/postgresql/postgres/bld
./bin/initdb -D ./data
./bin/pg_ctl -D ./data start
./bin/createdb x
./bin/createdb test_init
./bin/createdb test123
./bin/pg_ctl -D ./data stop
mkdir -p data_all
mv data data_all/ori_data

echo "Finished setup\n"
