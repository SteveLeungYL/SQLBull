#!/bin/bash -e
cd "$(dirname "$0")"/../docker

## Release code. Remove all intermediate steps to save hard drive space.
echo "Running Docker build."
sudo docker build --rm=true -f ./Dockerfile_no_fuzzer -t parserfuzz_mysql_no_fuzzer .
