SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
PROGS = $(patsubst %.cpp,%,$(SRCS))

# Tests if the parser builds correctly and doesn't contain conflicts.

PREFIX     ?= /usr/local
BIN_PATH    = $(PREFIX)/bin
HELPER_PATH = $(PREFIX)/lib/afl
DOC_PATH    = $(PREFIX)/share/doc/afl
MISC_PATH   = $(PREFIX)/share/afl

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

export CXX = g++
export DEBUG = -g
export OPT = -O0
export ASAN_FLAGS = -fsanitize=address
export DB_NAME = cockroachdb

CPPFLAGS = -std=c++17 -fpermissive -static-libstdc++ $(DEBUG) $(OPT) $(ASAN_FLAGS)
LDFLAGS = -ldl -lrt

COMPILE.c = $(CXX) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -std=c++17 -c -fPIC -D$(DB_NAME)

COMM_SRCS := $(wildcard ../dbms_specific/$(DB_NAME)/*.cpp)
COMM_SRCS += $(wildcard ../yacc/*.cpp)
COMM_SRCS += $(wildcard ../utils/*.cpp)
COMM_SRCS += $(wildcard ../scripts/*.cpp)
COMM_SRCS += $(wildcard ../rsg_generator/*.cpp)
COMM_SRCS += $(wildcard ../query_sequence/*.cpp)
COMM_SRCS += $(wildcard ../IR/*.cpp)
COMM_SRCS += $(wildcard ../grammar_expr_filter/*.cpp)
COMM_SRCS += $(wildcard ../feedback_mapper/*.cpp)
COMM_SRCS += $(wildcard ../query_importer/*.cpp)
COMM_OBJS = $(patsubst %cpp,%o,$(COMM_SRCS))
MAIN_FUZZER_OBJS = $(COMM_OBJS)

all: $(PROGS)
	echo $(PROGS)

clean:
	rm -rf *.o $(PROGS) .deps

$(PROGS): %: %.o
	$(CXX) $(CPPFLAGS) $(MAIN_FUZZER_OBJS) $< -o $@ $(LDFLAGS)
	cp $@ ../

$(OBJS): %.o: %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):

.PHONY: $(DEPDIR)

include $(wildcard $(DEPFILES))
