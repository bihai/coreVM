# The MIT License (MIT)

# Copyright (c) 2015 Yanzheng Li

# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# TODO: [COREVM-161] Cleanup final build dependencies in Makefile

TOP_DIR=$(CURDIR)

BIN=bin
SRC=src
TESTS=tests
TOOLS=./tools
PYTHON_DIR=./python

CXX=`which clang++`
CFLAGS=-Wall -std=c++11 -I$(TOP_DIR)/$(SRC)
EXTRA_CFLAGS=-Wno-deprecated

LIBGTEST=/usr/lib/libgtest.a
LIBSNEAKER=/usr/local/lib/libsneaker.a
LIBRARIES=$(LIBGTEST)

LIBCOREVM=libcorevm.a
LFLAGS=-lsneaker -lpthread

COREVM=coreVM

AR=ar
ARFLAGS=rvs

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS += -D LINUX
	LFLAGS += -lboost_system -lboost_regex -lboost_program_options
endif
ifeq ($(UNAME_S), Darwin)
	CFLAGS += -arch x86_64 -DGTEST_HAS_TR1_TUPLE=0
	LFLAGS += -lboost_system-mt -lboost_regex-mt -lboost_program_options
endif

PYTHON=`which python`
BOOTSTRAP_TESTS=bootstrap_tests.py


export GTEST_COLOR=true


include src/include.mk
include tools/include.mk
include tests/include.mk


BUILD_DIR = $(TOP_DIR)/build

OBJECTS = $(patsubst $(TOP_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SOURCES))
EXECS = $(patsubst $(TOP_DIR)/%.cc, %, $(EXEC_SOURCES))
TOOLS_EXECS = $(patsubst $(TOP_DIR)/%.cc, %, $(TOOL_EXEC_SOURCES))
TEST_OBJECTS = $(patsubst $(TOP_DIR)/%.cc,$(BUILD_DIR)/%.o,$(TEST_SOURCES))


$(BUILD_DIR)/%.o: $(TOP_DIR)/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(EXTRA_CFLAGS) -c $(TOP_DIR)/$*.cc -o $@


$(LIBCOREVM): $(OBJECTS)
	mkdir -p $(@D)
	@find . -name "*.o" | xargs $(AR) $(ARFLAGS) $(LIBCOREVM)
	@echo "\033[35mGenerated $(LIBCOREVM)"


$(COREVM): $(LIBCOREVM)
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(EXTRA_CFLAGS) src/corevm/main.cc -o $(COREVM) $^ $(LFLAGS)
	@echo "\033[35mGenerated $(COREVM)"


$(TOOLS): $(LIBCOREVM)
	mkdir -p $(@D)
	-for f in $(TOOLS)/*.cc; do ($(CXX) $(CFLAGS) $(EXTRA_CFLAGS) $$f -o $$f.out $^ $(LFLAGS);); done


$(PYTHON): $(COREVM)
	@$(PYTHON) $(PYTHON_DIR)/$(BOOTSTRAP_TESTS)


.PHONY: all
all: $(LIBCOREVM) $(COREVM) $(TOOLS) $(PYTHON)


.PHONY: $(TESTS)
$(TESTS): $(TEST_OBJECTS)
	mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(EXTRA_CFLAGS) $(TEST_OBJECTS) -o tests.out libcorevm.a $(LFLAGS) -lgtest
	@echo "\033[32mTests run completed...\033[39m";



.PHONY: clean
clean:
	@-rm -rf $(BUILD_DIR)
	@-rm -rf $(LIBCOREVM)
	@-rm -rf $(COREVM)
