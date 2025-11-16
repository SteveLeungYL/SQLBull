#!/bin/bash -e

for i in $(seq $1 $2)
do
    sudo docker run --memory="8g" --memory-swap="8g" --detach -i --rm --name postgresql_testing_$i parserfuzz_postgresql /bin/bash /home/postgresql/scripts/run_sqlbull_postgresql_fuzzing_helper.sh $i
done