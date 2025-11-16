FROM ubuntu:20.04
MAINTAINER "XXX-Security-Universe"

ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN chmod 777 /tmp

RUN apt -y upgrade \
&& apt -y update \
&& apt -y install bison  \
&& apt -y install flex  \
&& apt -y install git  \
&& apt -y install make  \
&& apt -y install cmake  \
&& apt -y install build-essential  \
&& apt -y install aptitude \
&& aptitude -y install gcc-9  \
&& aptitude -y install g++-9  \
&& apt -y install clang \
&& apt -y install xinetd  \
&& apt -y install libreadline-dev  \
&& apt -y install zlib1g-dev  \
&& apt -y install gdb  \
&& apt -y install vim  \
&& apt -y install tmux  \
&& apt -y install python3 \
&& apt -y install python3-pip \
&& apt -y install libxml2-dev  \
&& apt -y install libxslt-dev  \
&& apt -y install libssl-dev  \
&& apt -y install libxml2-utils  \
&& apt -y install xsltproc \
&& apt install -y libpq-dev \
&& apt -y install screen \
&& apt -y install watch \
&& apt -y install unzip \
&& apt -y install wget \
&& apt -y install curl \
&& apt -y install autoconf \
&& apt -y install yarn \
&& apt -y install clang-format \
&& apt -y install npm \
&& apt -y install llvm \
&& apt -y install clang \
&& apt install -y g++-9 \
&& apt install -y gcc-9 \
&& apt install -y htop \
&& apt install -y zip \
&& apt install -y libresolv-wrapper \
&& npm install -g @bazel/bazelisk

RUN useradd -ms /bin/bash cockroachdb

USER cockroachdb

RUN pip3 install libtmux loguru

USER root
RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash
RUN apt update
RUN apt -y install nodejs

WORKDIR /home/cockroachdb

USER cockroachdb
RUN curl --compressed -o- -L https://yarnpkg.com/install.sh | bash

# Download the original Go library. Set it up.

#*******************************************************************************
# REMEMBER TO UPDATE THIS LINK WITH RESPECT TO COCKROACHDB WORKSPACE!!!!!!!!!!!!!!!!!!!
# WORKSPACE, check which version of Go is supported in go_download_sdk.
RUN wget https://storage.googleapis.com/public-bazel-artifacts/go/20241105-184020/go1.22.8.linux-amd64.tar.gz
RUN tar -xf go1.22.8.linux-amd64.tar.gz
USER root
RUN cp -r go /usr/local/go

# for the original go, we can update as we want.
# used for compiling the content in the instrumented go std library.
RUN mkdir -p go_ori
WORKDIR /home/cockroachdb/go_ori
RUN wget https://go.dev/dl/go1.23.0.linux-amd64.tar.gz
RUN tar -xf go1.23.0.linux-amd64.tar.gz && cp -r go /root/go

USER cockroachdb
RUN echo 'export PATH="/usr/local/go/bin:$PATH"' >> ~/.bashrc
RUN echo 'export PATH="/home/cockroachdb/.yarn/bin:$PATH"' >> ~/.bashrc
ENV PATH="/usr/local/go/bin:$PATH"
ENV PATH="/home/cockroachdb/.yarn/bin:$PATH"

# No need for AFL. CockroachDB is a Golang program. Construct the GOPATH folder.

USER cockroachdb
RUN mkdir -p /home/cockroachdb/go_projects/src/github.com/cockroachdb
WORKDIR /home/cockroachdb/go_projects/src/github.com/cockroachdb

# CockroachDB source pull setup
RUN git clone https://github.com/cockroachdb/cockroach.git
RUN cp -r cockroach cockroach_ori
RUN echo 'export GOPATH="/home/cockroachdb/go_projects"' >> ~/.bashrc
ENV GOPATH="/home/cockroachdb/go_projects"

# Move the CockroachDB Package Plugin into the Docker Image.
USER root
COPY CockroachDB_pkg_plugin /home/cockroachdb/CockroachDB_pkg_plugin
RUN chown -R cockroachdb:cockroachdb /home/cockroachdb/CockroachDB_pkg_plugin

# Compile the latest version of CockroachDB, prep for coverage instrumentation.
USER cockroachdb
WORKDIR /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach
# this is official release on date March 13th, 2024
RUN git checkout v24.3.1
#RUN bazel build pkg/cmd/cockroach-short && cp ./_bazel/bin/pkg/cmd/cockroach-short/cockroach-short_/cockroach-short ./cockroachdb

## Import the covtest unit test entry
USER cockroachdb
WORKDIR /home/cockroachdb
RUN cp -r ./CockroachDB_pkg_plugin/covtest /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach/pkg/

# Helper features from std for the goInst.
USER root
WORKDIR /usr/local/go/src/cmd
RUN cp -r ./internal/edit ./edit

# Introduce the globalcov module to std.
USER root
RUN cp -r /home/cockroachdb/CockroachDB_pkg_plugin/globalcov /usr/local/go/src/globalcov
WORKDIR /usr/local/go/src
ENV GOROOT_BOOTSTRAP="/root/go"
RUN bash make.bash
RUN go get golang.org/x/net@master && go mod tidy && go mod vendor
# Remove the suspeciously present linux_arm64 tool folder, could cause compilation failure.
# This problem seems only occurs in the go arm64 host, amd 64 not affected.
# RUN rm -rf /usr/local/go/pkg/tool/linux_arm64

#
# Instrument the CockroachDB source code.
USER cockroachdb
WORKDIR /home/cockroachdb/
RUN cp ./CockroachDB_pkg_plugin/inst_script.py /home/cockroachdb/go_projects/src/github.com/cockroachdb/
WORKDIR /home/cockroachdb/CockroachDB_pkg_plugin/goInst/
RUN go build
RUN cp ./goInstr /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach/
WORKDIR /home/cockroachdb/go_projects/src/github.com/cockroachdb/
RUN python3 inst_script.py

USER cockroachdb
WORKDIR /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach/
RUN bazel build pkg/cmd/cockroach-short && cp _bazel/bin/pkg/cmd/cockroach-short/cockroach-short_/cockroach-short ./cockroach
RUN bazel build pkg/covtest && cp _bazel/bin/pkg/covtest/covtest_/covtest ./covtest
USER root

### Finished setup of the SQLRight Validity Configurations.
#
## Setup the fuzzing context.
USER root
RUN mkdir /home/cockroachdb/fuzzing/

COPY fuzz_root /home/cockroachdb/fuzzing/fuzz_root/
RUN mkdir -p /home/cockroachdb/fuzzing/fuzz_root/outputs
RUN mkdir -p /home/cockroachdb/fuzzing/Bug_Analysis
RUN mkdir -p /home/cockroachdb/fuzzing/Bug_Analysis/bug_samples

RUN cp /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach/covtest /home/cockroachdb/fuzzing/fuzz_root/covtest.test

USER root
RUN chown -R cockroachdb:cockroachdb /home/cockroachdb/fuzzing/
#
## Setup the Docker run script
USER root
COPY scripts /home/cockroachdb/scripts
RUN chown -R cockroachdb:cockroachdb /home/cockroachdb/scripts

# Compile and Copy the new RSG_CPP into the fuzzing location.
COPY rsg_cpp /home/cockroachdb/rsg_cpp
USER root
RUN chown -R cockroachdb:cockroachdb /home/cockroachdb/rsg_cpp
USER cockroachdb
WORKDIR /home/cockroachdb/rsg_cpp/scripts
RUN cp /home/cockroachdb/go_projects/src/github.com/cockroachdb/cockroach/pkg/sql/parser/sql.y \
          /home/cockroachdb/rsg_cpp/scripts/assets/cockroach_sql.y
RUN python3 remove_action.py assets/cockroach_sql.y && cp assets/cockroach_sql_modi.y ../ && cp assets/cockroach_sql_modi.y /home/cockroachdb/fuzzing/fuzz_root/
RUN python3 generate_ir_types_cockroachdb.py && cp /home/cockroachdb/rsg_cpp/scripts/assets/ir_types_mapping.txt /home/cockroachdb/fuzzing/fuzz_root/ir_types_mapping.txt && cp /home/cockroachdb/rsg_cpp/scripts/assets/ir_types_custom.h /home/cockroachdb/rsg_cpp/headers/ir_types_custom.h
WORKDIR /home/cockroachdb/rsg_cpp/
RUN make cockroachdb -j $(nproc) && cp parserfuzz /home/cockroachdb/fuzzing/fuzz_root/parserfuzz

# Final entry
USER root
WORKDIR /home/cockroachdb/fuzzing/fuzz_root

COPY fuzzer_version /home/cockroachdb/fuzzer_version
RUN chown cockroachdb:cockroachdb /home/cockroachdb/fuzzer_version
