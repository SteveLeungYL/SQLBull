FROM ubuntu:20.04 AS parserfuzz_mysql_tools
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

COPY AFL /home/mysql/AFL

RUN useradd -ms /bin/bash mysql

RUN chown -R mysql:mysql /home/mysql/

USER mysql
WORKDIR /home/mysql/AFL
RUN AFL_NO_X86=1 make
WORKDIR /home/mysql/AFL/llvm_mode
RUN make

## ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_mysql_tools AS parserfuzz_mysql_source_build

# build MySQL instrumented by afl-clang-fast
USER mysql
WORKDIR /home/mysql/
RUN wget https://github.com/mysql/mysql-server/archive/refs/tags/mysql-8.0.40.tar.gz
RUN tar -xf mysql-8.0.40.tar.gz
RUN mv mysql-server-mysql-8.0.40 mysql-server
WORKDIR /home/mysql/mysql-server
RUN mkdir bld
WORKDIR /home/mysql/mysql-server/bld
ENV CC=/home/mysql/AFL/afl-clang-fast
ENV CXX=/home/mysql/AFL/afl-clang-fast++
ENV CPP=/home/mysql/AFL/afl-clang-fast++
RUN CC=/home/mysql/AFL/afl-clang-fast CXX=/home/mysql/AFL/afl-clang-fast++ CPP=/home/mysql/AFL/afl-clang-fast++ cmake .. -DDOWNLOAD_BOOST=1 -DWITH_BOOST=../boost -DWITH_UNIT_TESTS=OFF -DWITH_DEBUG=1 -DWITH_ASAN=1 -DCPACK_MONOLITHIC_INSTALL=1
RUN make -j$(nproc)

ENV CC=
ENV CXX=
ENV CPP=

# ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_mysql_source_build AS parserfuzz_main_fuzzer_build 

USER root
COPY rsg_cpp /home/mysql/rsg_cpp
RUN chown -R mysql:mysql /home/mysql/rsg_cpp

USER mysql
WORKDIR /home/mysql/rsg_cpp
RUN cp /home/mysql/mysql-server/sql/sql_yacc.yy /home/mysql/rsg_cpp/scripts/assets/mysql_grammar.y
RUN make clean && make mysql -j $(nproc)


# ---------------------------------------------------------------------------------------------------------------
FROM parserfuzz_main_fuzzer_build AS mysql_config_setup 
USER root
COPY scripts /home/mysql/scripts
RUN chown -R mysql:mysql /home/mysql/scripts

USER mysql
WORKDIR /home/mysql/mysql-server/bld/
RUN mkdir data
RUN bin/mysqld --initialize-insecure --user=mysql --datadir=data
RUN bin/mysql_ssl_rsa_setup --datadir=data # Only available in MySQL 8.0.*. Delete this line for MySQL 9.0.*.
RUN mkdir data_all
RUN mv data data_all/ori_data

WORKDIR /home/mysql/scripts
RUN python3 setup_database.py

# ---------------------------------------------------------------------------------------------------------------
FROM mysql_config_setup AS fuzzing_env_setup
USER root
RUN mkdir -p /home/mysql/fuzzing && mkdir -p /home/mysql/fuzzing/Bug_Analysis
COPY fuzz_root /home/mysql/fuzzing/fuzz_root
RUN chown -R mysql:mysql /home/mysql/fuzzing

USER mysql
WORKDIR /home/mysql/fuzzing/fuzz_root
RUN cp /home/mysql/rsg_cpp/parserfuzz ./ && cp /home/mysql/rsg_cpp/scripts/assets/mysql_grammar_modi.y ./ && cp /home/mysql/rsg_cpp/scripts/assets/ir_types_mapping.txt ./

# This is for fuzzing with AFLplusplus.
# RUN AFL_DEBUG=1 __AFL_SHM_ID=1234 /home/mysql/mysql-server/bld/bin/mysqld 2>&1 | grep "__afl_map_size" | tail -n 1 | cut -d"," -f8 | cut -d" " -f 3 > /home/mysql/fuzzing/fuzz_root/shm_size

USER root
RUN echo "[mysqld]\nsql_mode = \"NO_ENGINE_SUBSTITUTION\"\nmax_allowed_packet=16M\n" > /etc/mysql/conf.d/mysql.cnf

WORKDIR /home/mysql/fuzzing/fuzz_root