#!/bin/bash -e

for i in $(seq $1 $2)
do
    sudo docker run --detach -i --rm --name mysql_testing_$i parserfuzz_mysql /bin/bash /home/mysql/scripts/run_sqlright_mysql_fuzzing_helper.sh $i
done