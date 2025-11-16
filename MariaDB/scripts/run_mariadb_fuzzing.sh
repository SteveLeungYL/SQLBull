#!/bin/bash -e

for i in $(seq $1 $2)
do
    sudo docker run --detach -i --rm --memory="2g" --memory-swap="2g" --name mariadb_testing_$i parserfuzz_mariadb /bin/bash /home/mariadb/scripts/run_mariadb_fuzzing_helper.sh $i
done