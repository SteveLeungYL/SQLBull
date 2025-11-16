FROM ubuntu:20.04 AS parserfuzz_mariadb_tools
MAINTAINER "XXX-Security-Universe"

ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN chmod 777 /tmp

RUN apt -y update && apt -y install bison build-essential cmake flex gdb git libncurses5-dev libreadline-dev libssl-dev make pkg-config python3 python3-pip tmux vim xinetd zlib1g-dev  screen watch unzip wget libtirpc-dev htop libpthread-stubs0-dev

# Install mysql-client related libraries, for the compilation of sqlright.
RUN pip install libtmux
RUN pip3 install psutil
RUN pip3 install mysql-connector-python mysql-connector-python
RUN apt -y install libmysqlclient-dev
RUN pip3 install mysqlclient

RUN apt -y install g++
RUN apt -y install gcc
RUN apt -y install llvm
RUN apt -y install clang

RUN apt -y install libnuma-dev
RUN apt -y install pkg-config


RUN useradd -ms /bin/bash mariadb 

COPY AFL /home/mariadb/AFL
RUN chown -R mariadb:mariadb /home/mariadb/

USER mariadb 
WORKDIR /home/mariadb/AFL
RUN make
WORKDIR /home/mariadb/AFL/llvm_mode
RUN make

## ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_mariadb_tools AS parserfuzz_mariadb_source_build

# build mariadb instrumented by afl-clang-fast
USER mariadb
WORKDIR /home/mariadb/
RUN git clone https://github.com/MariaDB/server.git && mv server mariadb-server
WORKDIR /home/mariadb/mariadb-server
RUN git checkout mariadb-11.7.2 && git submodule update --init --recursive
RUN mkdir bld
WORKDIR /home/mariadb/mariadb-server/bld
ENV CC=/home/mariadb/AFL/afl-clang-fast
ENV CXX=/home/mariadb/AFL/afl-clang-fast++
ENV CPP=/home/mariadb/AFL/afl-clang-fast++
RUN CC=/home/mariadb/AFL/afl-clang-fast CXX=/home/mariadb/AFL/afl-clang-fast++ CPP=/home/mariadb/AFL/afl-clang-fast++ cmake .. -DCMAKE_INSTALL_PREFIX=$(pwd) -DWITH_ASAN=ON -DCMAKE_BUILD_TYPE=Debug
RUN make install -j$(nproc)

ENV CC=
ENV CXX=
ENV CPP=

# ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_mariadb_source_build AS parserfuzz_main_fuzzer_build 

USER root
COPY rsg_cpp /home/mariadb/rsg_cpp
RUN chown -R mariadb:mariadb /home/mariadb/rsg_cpp

USER mariadb
WORKDIR /home/mariadb/rsg_cpp
RUN cp /home/mariadb/mariadb-server/sql/sql_yacc.yy /home/mariadb/rsg_cpp/scripts/assets/mariadb_grammar.y
RUN make clean && make mariadb -j $(nproc)


# ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_main_fuzzer_build AS mariadb_config_setup 
USER root
COPY scripts /home/mariadb/scripts
RUN chown -R mariadb:mariadb /home/mariadb/scripts && chown -R mariadb:mariadb /tmp && chown -R mariadb:mariadb /var

USER mariadb
WORKDIR /home/mariadb/mariadb-server/bld/
RUN mkdir data
RUN ./scripts/mariadb-install-db  --user=mariadb
RUN mkdir data_all
RUN mv data data_all/ori_data

WORKDIR /home/mariadb/scripts
RUN python3 setup_database.py

# ---------------------------------------------------------------------------------------------------------------
FROM mariadb_config_setup AS fuzzing_env_setup
USER root
RUN mkdir -p /home/mariadb/fuzzing && mkdir -p /home/mariadb/fuzzing/Bug_Analysis
COPY fuzz_root /home/mariadb/fuzzing/fuzz_root
RUN chown -R mariadb:mariadb /home/mariadb/fuzzing

USER mariadb
WORKDIR /home/mariadb/fuzzing/fuzz_root
RUN cp /home/mariadb/rsg_cpp/parserfuzz ./ && cp /home/mariadb/rsg_cpp/scripts/assets/mariadb_grammar_modi.y ./ && cp /home/mariadb/rsg_cpp/scripts/assets/ir_types_mapping.txt ./

# This is for fuzzing with AFLplusplus.
# RUN AFL_DEBUG=1 __AFL_SHM_ID=1234 /home/mariadb/mariadb-server/bld/bin/mariadbd 2>&1 | grep "__afl_map_size" | tail -n 1 | cut -d"," -f8 | cut -d" " -f 3 > /home/mariadb/fuzzing/fuzz_root/shm_size

USER root
RUN echo "[mysqld]\nsql_mode = \"NO_ENGINE_SUBSTITUTION\"\nmax_allowed_packet=16M\n" > /etc/mysql/conf.d/mysql.cnf

WORKDIR /home/mariadb/fuzzing/fuzz_root