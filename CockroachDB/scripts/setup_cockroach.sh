#!/bin/bash -e
cd "$(dirname "$0")"/../docker

rm -rf ./CockroachDB_pkg_plugin/rsg &> /dev/null
rm -rf ./CockroachDB_pkg_plugin/rsg_cpp &> /dev/null
rm -rf ./rsg_cpp &> /dev/null
cp -r ../../Common_Tootls/rsg ./CockroachDB_pkg_plugin/rsg
cp -r ../../Common_Tootls/rsg_cpp ./rsg_cpp
cd ./rsg_cpp/scripts/ && python3 generate_ir_types_cockroachdb.py && cp ./assets/ir_types_custom.h ../headers/ir_types_custom.h && cd ../../

git show HEAD~2 --pretty=format:"%h" --no-patch &> ./fuzzer_version

## For debug purpose, keep all intermediate steps to fast reproduce the run results.
# sudo docker build --rm=false -f ./Dockerfile -t parserfuzz_cockroach .

## Release code. Remove all intermediate steps to save hard drive space.
ARCH=$(uname -m)
if [ $ARCH = "x86_64" ]; then
  echo "Running x86-64 Docker build."
  sudo docker build --rm=true -f ./Dockerfile -t sqlbull_cockroach .
else
  echo "Running ARM64 Docker build."
  sudo docker build --rm=true -f ./Dockerfile_ARM64 -t sqlbull_cockroach .
fi
