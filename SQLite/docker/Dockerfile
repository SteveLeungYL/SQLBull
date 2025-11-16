FROM ubuntu:20.04 AS fundamental

ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN chmod 777 /tmp

RUN apt -y upgrade && \
apt -y update && \
apt -y install bison  && \
apt -y install flex  && \
apt -y install git  && \
apt -y install make  && \
apt -y install cmake  && \
apt -y install build-essential  && \
apt -y install clang && \
apt -y install libreadline-dev  && \
apt -y install zlib1g-dev  && \
apt -y install gdb  && \
apt -y install vim  && \
apt -y install tmux  && \
apt -y install python3 && \
apt -y install python3-pip && \
apt -y install screen && \
apt -y install watch && \
apt -y install unzip && \
apt -y install wget && \
apt -y install tcl && \
apt -y install llvm && \
apt -y install clang && \
apt -y install tclsh && \
apt install -y g++-9 && \
apt install -y gcc-9 && \
apt install -y lemon && \
apt install -y zip && \
apt install -y neovim && \
pip3 install gitpython

# Setup sqlite user.
RUN useradd -ms /bin/bash sqlite
USER sqlite
RUN pip3 install libtmux loguru

# Build AFL.
USER sqlite
WORKDIR /home/sqlite
RUN git clone https://github.com/google/AFL.git
WORKDIR /home/sqlite/AFL
RUN sed -i  's/#define MAP_SIZE_POW2       16/#define MAP_SIZE_POW2       18/' config.h
RUN make
WORKDIR /home/sqlite/AFL/llvm_mode
# ENV LLVM_CONFIG=llvm-config-6.0
RUN make

################################################################################
# Build SQLite3
FROM fundamental AS sqlite3_build

# Setup the helper scripts within the container, contains the script to modify the SQLite shell.
USER root
WORKDIR /home/sqlite
COPY scripts /home/sqlite/scripts
RUN chown -R sqlite:sqlite /home/sqlite/scripts

USER sqlite
WORKDIR /home/sqlite
RUN git clone https://github.com/sqlite/sqlite.git
RUN cp -r sqlite sqlite_ori

## Build instrumented SQLite3
WORKDIR /home/sqlite/sqlite
# Always test the latest version.
RUN git checkout master
# For Fossil: RUN git checkout trunk
RUN python3 /home/sqlite/scripts/modify_sqlite_shell.py
ENV CXX=/home/sqlite/AFL/afl-clang-fast++
ENV CC=/home/sqlite/AFL/afl-clang-fast
ENV CFLAGS="-fsanitize=address -fsanitize=undefined -g -DSQLITE_DEBUG -DSQLITE_ENABLE_TREETRACE -DSQLITE_ENABLE_WHERETRACE -DSQLITE_ENABLE_CURSOR_HINTS -DSQLITE_COUNTOFVIEW_OPTIMIZATION -DSQLITE_ENABLE_STAT4 -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_ORDERED_SET_AGGREGATES -DSQLITE_ENABLE_RTREE"
ENV CXXFLAGS="-fsanitize=address -fsanitize=undefined -g -DSQLITE_DEBUG -DSQLITE_ENABLE_TREETRACE -DSQLITE_ENABLE_WHERETRACE -DSQLITE_ENABLE_CURSOR_HINTS -DSQLITE_COUNTOFVIEW_OPTIMIZATION -DSQLITE_ENABLE_STAT4 -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_ORDERED_SET_AGGREGATES -DSQLITE_ENABLE_RTREE"
ENV LDFLAGS="-fsanitize=address -fsanitize=undefined"
RUN ./configure --enable-debug --enable-all --disable-shared
RUN make -j$(nproc); make; exit 0;

## Build uninstrumented and unmodified SQLite3
WORKDIR /home/sqlite/sqlite_ori
RUN git checkout master
# For Fossil: RUN git checkout trunk
ENV CXX=g++-9
ENV CC=gcc-9
RUN ./configure --enable-debug --enable-all --disable-shared
RUN make -j$(nproc); make; exit 0;

ENV CFLAGS=
ENV CXXFLAGS=
ENV LDFLAGS=

################################################################################
# Setup the ParserFuzz source code.
FROM sqlite3_build AS parserfuzz_build

# Build the ParserFuzz source code.
USER root
COPY rsg_cpp /home/sqlite/rsg_cpp
RUN chown -R sqlite:sqlite /home/sqlite/rsg_cpp

USER sqlite
# Borrow the latest version of SQLite parser from the build context.
RUN rm -f /home/sqlite/rsg_cpp/scripts/assets/sqlite_ori_parser.y && cp /home/sqlite/sqlite_ori/src/parse.y /home/sqlite/rsg_cpp/scripts/assets/sqlite_ori_parser.y

WORKDIR /home/sqlite/rsg_cpp
RUN make clean && make sqlite -j$(nproc)

################################################################################
# Setup the fuzzing context
FROM parserfuzz_build AS sqlite3_fuzzing
USER sqlite
WORKDIR /home/sqlite
RUN mkdir -p fuzzing

USER root
WORKDIR /home/sqlite/fuzzing
COPY fuzz_root /home/sqlite/fuzzing/fuzz_root
WORKDIR /home/sqlite/fuzzing/fuzz_root
RUN cp /home/sqlite/rsg_cpp/scripts/assets/sqlite_parser.y /home/sqlite/rsg_cpp/scripts/assets/ir_types_mapping.txt /home/sqlite/fuzzing/fuzz_root
RUN cp /home/sqlite/sqlite/sqlite3 /home/sqlite/fuzzing/fuzz_root/sqlite3
RUN cp /home/sqlite/rsg_cpp/parserfuzz /home/sqlite/fuzzing/fuzz_root/parserfuzz
RUN chown -R sqlite:sqlite /home/sqlite/fuzzing/

USER sqlite
WORKDIR /home/sqlite/fuzzing
RUN mkdir -p Bug_Analysis

# Deprecated below.
# Setup the SQLite bisecting cached binaries.
#USER root
#COPY sqlite_bisecting_binary /home/sqlite/sqlite_bisecting_binary
#RUN chown -R sqlite:sqlite /home/sqlite/sqlite_bisecting_binary

# Setup the SQLite bisecting environment
#USER root
#COPY bisecting /home/sqlite/bisecting
#RUN chown -R sqlite:sqlite /home/sqlite/bisecting

# Also, setup the bisecting sqlite folder.
#USER sqlite
#WORKDIR /home/sqlite
#RUN git clone https://github.com/sqlite/sqlite.git ./sqlite_bisecting_binary
#WORKDIR /home/sqlite/sqlite_bisecting_binary
#RUN mkdir -p bld


# Entry workdir.
USER root
WORKDIR /home/sqlite/fuzzing/fuzz_root
