SRCS = $(wildcard *.cpp)
SUB_DIRS := yacc rsg_generator grammar_expr_filter IR utils dbms_specific feedback_mapper query_importer query_sequence

# Tests if the parser builds correctly and doesn't contain conflicts.

PREFIX     ?= /usr/local
BIN_PATH    = $(PREFIX)/bin
HELPER_PATH = $(PREFIX)/lib/afl
DOC_PATH    = $(PREFIX)/share/doc/afl
MISC_PATH   = $(PREFIX)/share/afl

MAIN_FUZZER_FLAGS += -w -DAFL_PATH=\"$(HELPER_PATH)\" -DDOC_PATH=\"$(DOC_PATH)\" \
			-DBIN_PATH=\"$(BIN_PATH)\"

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

export CXX = g++
export DEBUG = -g
export OPT = -O3
export CPPFLAGS = -std=c++17 -fpermissive -static-libstdc++ $(DEBUG) $(OPT) $(ASAN_FLAGS) -D$(DB_NAME) $(DB_FLAGS)
export DB_NAME = cockroachdb
export LDFLAGS = -ldl -lrt $(DB_LDFLAGS)

COMPILE.c = $(CXX) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -std=c++17 -c -fPIC

COMM_SRCS := $(wildcard dbms_specific/$(DB_NAME)/*.cpp)
COMM_SRCS += $(wildcard yacc/*.cpp)
COMM_SRCS += $(wildcard utils/*.cpp)
COMM_SRCS += $(wildcard scripts/*.cpp)
COMM_SRCS += $(wildcard rsg_generator/*.cpp)
COMM_SRCS += $(wildcard query_sequence/*.cpp)
COMM_SRCS += $(wildcard IR/*.cpp)
COMM_SRCS += $(wildcard grammar_expr_filter/*.cpp)
COMM_SRCS += $(wildcard feedback_mapper/*.cpp)
COMM_SRCS += $(wildcard query_importer/*.cpp)
COMM_OBJS = $(patsubst %cpp,%o,$(COMM_SRCS))
MAIN_FUZZER_OBJS = $(COMM_OBJS) parserfuzz.o

no_db_name:
	echo "Usage: make <DB_name>"

cockroachdb: DB_NAME = cockroachdb
cockroachdb:
	$(MAKE) -e main_entry
cockroachdb_debug: DEBUG = -g
cockroachdb_debug: OPT = -O0
cockroachdb_debug: DB_NAME = cockroachdb
cockroachdb_debug:
	$(MAKE) -e cockroachdb
cockroachdb_asan: ASAN_FLAGS=-fsanitize=address
cockroachdb_asan: cockroachdb_debug

duckdb: DB_NAME = duckdb
duckdb:
	$(MAKE) -e main_entry
duckdb_debug: DEBUG = -g
duckdb_debug: OPT = -O0
duckdb_debug: DB_NAME = duckdb
duckdb_debug:
	$(MAKE) -e duckdb
duckdb_asan: ASAN_FLAGS=-fsanitize=address
duckdb_asan: duckdb_debug

sqlite: DB_NAME = sqlite
sqlite:
	$(MAKE) -e main_entry
sqlite_debug: DEBUG = -g
sqlite_debug: OPT = -O0
sqlite_debug: DB_NAME = sqlite
sqlite_debug:
	$(MAKE) -e sqlite
sqlite_asan: ASAN_FLAGS=-fsanitize=address
sqlite_asan: sqlite_debug

mysql: DB_NAME = mysqldb
mysql: DB_FLAGS=-I/usr/include/mysql -pthread
mysql: DB_LDFLAGS=-lmysqlclient -lpthread
mysql:
	$(MAKE) -e main_entry
mysql_debug: DEBUG = -g
mysql_debug: OPT = -O0
mysql_debug: DB_NAME = mysqldb
mysql_debug: DB_FLAGS=-I/usr/include/mysql -pthread
mysql_debug: DB_LDFLAGS=-lmysqlclient -lpthread
mysql_debug:
	$(MAKE) -e mysql
mysql_asan: ASAN_FLAGS=-fsanitize=address
mysql_asan: DB_FLAGS=-I/usr/include/mysql -pthread
mysql_asan: DB_LDFLAGS=-lmysqlclient -lpthread
mysql_asan: mysql_debug


mariadb: DB_NAME = mariadb
mariadb: DB_FLAGS=-I/usr/include/mysql -pthread
mariadb: DB_LDFLAGS=-lmysqlclient -lpthread
mariadb:
	$(MAKE) -e main_entry
mariadb_debug: DEBUG = -g
mariadb_debug: OPT = -O0
mariadb_debug: DB_NAME = mariadb
mariadb_debug: DB_FLAGS=-I/usr/include/mysql -pthread
mariadb_debug: DB_LDFLAGS=-lmysqlclient -lpthread
mariadb_debug:
	$(MAKE) -e mariadb
mariadb_asan: ASAN_FLAGS=-fsanitize=address
mariadb_asan: DB_FLAGS=-I/usr/include/mysql -pthread
mariadb_asan: DB_LDFLAGS=-lmysqlclient -lpthread
mariadb_asan: mariadb_debug

postgresql: DB_NAME = postgresql
postgresql: DB_FLAGS=-I/usr/include/postgresql -I/usr/local/pgsql/include -pthread
postgresql: DB_LDFLAGS=-lpq -L/usr/local/pgsql/lib
postgresql:
	$(MAKE) -e main_entry
postgresql_debug: DEBUG = -g
postgresql_debug: OPT = -O0
postgresql_debug: DB_NAME = postgresql
postgresql_debug: DB_FLAGS=-I/usr/include/postgresql -I/usr/local/pgsql/include -pthread
postgresql_debug: DB_LDFLAGS=-lpq -L/usr/local/pgsql/lib
postgresql_debug:
	$(MAKE) -e postgresql
postgresql_asan: ASAN_FLAGS=-fsanitize=address
postgresql_asan: DB_FLAGS=-I/usr/include/postgresql -I/usr/local/pgsql/include -pthread
postgresql_asan: DB_LDFLAGS=-lpq -L/usr/local/pgsql/lib
postgresql_asan: postgresql_debug

main_entry: headers/ir_types_custom.h
	@$(MAKE) -e all_srcs

headers/ir_types_custom.h:
	@$(MAKE) -C scripts $(DB_NAME)

all_srcs: parserfuzz unit_tests

clean:
	rm -rf *.output *.o .deps *.gch
	rm -f ./test_* parserfuzz
	rm -f headers/ir_types_custom.h
	@$(MAKE) clean -C yacc
	@$(MAKE) clean -C rsg_generator
	@$(MAKE) clean -C query_sequence
	@$(MAKE) clean -C grammar_expr_filter
	@$(MAKE) clean -C IR
	@$(MAKE) clean -C utils
	@$(MAKE) clean -C dbms_specific
	@$(MAKE) clean -C feedback_mapper
	@$(MAKE) clean -C query_importer
	@$(MAKE) clean -C unit_tests
	@$(MAKE) clean -C scripts

parserfuzz: headers/ir_types_custom.h parserfuzz.o $(SUB_DIRS)
	$(CXX) $(CPPFLAGS) $(MAIN_FUZZER_OBJS) -o $@ $(LDFLAGS)

parserfuzz.o: parserfuzz.cpp  $(DEPDIR)/parserfuzz.d | $(DEPDIR) $(SUB_DIRS)
	$(COMPILE.c) $(MAIN_FUZZER_FLAGS) $(OUTPUT_OPTION) $<

%.o: %.cpp $(SRCS)  $(DEPDIR)/%.d | $(DEPDIR) $(SUB_DIRS)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

unit_tests: headers/ir_types_custom.h $(SUB_DIRS)
	@$(MAKE) -C $@

yacc:
	@$(MAKE) -C $@

rsg_generator:
	@$(MAKE) -C $@

query_sequence:
	@$(MAKE) -C $@

grammar_expr_filter:
	@$(MAKE) -C $@

IR:
	@$(MAKE) -C $@

utils:
	@$(MAKE) -C $@

dbms_specific:
	@$(MAKE) -C $@ $(DB_NAME)

feedback_mapper:
	@$(MAKE) -C $@

query_importer:
	@$(MAKE) -C $@

asan: ASAN_FLAGS = -fsanitize=address
asan: DEBUG = -g
asan: OPT = -O0
asan:
	$(MAKE) -e

debug: DEBUG = -g
debug: OPT = -O0
debug:
	$(MAKE) -e

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

.PHONY: $(SUB_DIRS) $(DEPDIR) unit_tests

include $(wildcard $(DEPFILES))
