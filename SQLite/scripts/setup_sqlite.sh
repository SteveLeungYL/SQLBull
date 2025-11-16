#!/bin/bash -e
cd "$(dirname "$0")"/../docker

rm -rf ./rsg_cpp &> /dev/null

cp -r ../../Common_Tootls/rsg_cpp ./rsg_cpp
git show HEAD~2 --pretty=format:"%h" --no-patch &> ./fuzzer_version

## Release code. Remove all intermediate steps to save hard drive space.
ARCH=$(uname -m)
if [ $ARCH = "x86_64" ]; then
  echo "Running x86-64 Docker build."
  sudo docker build --rm=true -f ./Dockerfile -t sqlbull_sqlite .
else
  echo "Running ARM64 Docker build."
  sudo docker build --rm=true -f ./Dockerfile_ARM64 -t sqlbull_sqlite .
fi
