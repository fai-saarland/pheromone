DEBUG ?= no
WERROR ?= no


CC ?= gcc
CXX ?= g++
AR ?= ar
RANLIB ?= ranlib
PROTOC ?= protoc
GRPC_CPP_PLUGIN ?= /usr/bin/grpc_cpp_plugin
GRPC_PYTHON_PLUGIN ?= /usr/bin/grpc_python_plugin

ifeq '$(DEBUG)' 'yes'
  CXXFLAGS := -g
else
  CXXFLAGS += -O3 -pipe
endif
ifeq '$(WERROR)' 'yes'
  CXXFLAGS += -Werror
endif
CXXFLAGS += -Wall -pedantic
CXXFLAGS += -Iinclude
CXXFLAGS += $(shell pkg-config --cflags grpc++ protobuf)

ifeq '$(DEBUG)' 'yes'
  CFLAGS := -g -Iinclude -Wall -pedantic -Werror
else
  CFLAGS := -O3 -pipe -Iinclude -Wall -pedantic -Werror
endif

LDFLAGS += libpheromone.a
LDFLAGS += $(shell pkg-config --libs grpc++ protobuf)
LDFLAGS += -lstdc++


SRC  = policy_server
SRC += policy_client

PROTO  = fdr
PROTO += policy

GRPC_PROTO = policy

PROTO_H := $(foreach p,$(PROTO),src/rpc/$(p).pb.h)
PROTO_CC := $(foreach p,$(PROTO),src/rpc/$(p).pb.cc)
PROTO_PY := $(foreach p,$(PROTO),python/rpc/$(p)_pb2.py)

GRPC_PROTO_H := $(foreach p,$(GRPC_PROTO),src/rpc/$(p).grpc.pb.h)
GRPC_PROTO_CC := $(foreach p,$(GRPC_PROTO),src/rpc/$(p).grpc.pb.cc)
GRPC_PROTO_PY := $(foreach p,$(GRPC_PROTO),python/rpc/$(p)_pb2_grpc.py)

DEP  = Makefile
DEP += $(PROTO_H)
DEP += $(PROTO_CC)
DEP += $(GRPC_PROTO_H)
DEP += $(GRPC_PROTO_CC)

SRC += $(foreach p,$(PROTO),proto-$(p))
SRC += $(foreach p,$(GRPC_PROTO),grpc-$(p))

OBJS := $(foreach obj,$(SRC),.objs/$(obj).o)

TESTS  = policy_server
TESTS += policy_cat_fdr_task_fd
TESTS += policy_cat_fdr_init_state_op
TESTS := $(foreach t,$(TESTS),test/test_$(t))

all: libpheromone.a test

libpheromone.a: $(OBJS) $(DEP)
	rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

test: $(TESTS)

.objs/%.o: src/%.cc $(DEP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
.objs/proto-%.o: src/rpc/%.pb.cc src/rpc/%.pb.h $(DEP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
.objs/grpc-%.o: src/rpc/%.grpc.pb.cc src/rpc/%.grpc.pb.h $(DEP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
src/rpc/%.grpc.pb.cc src/rpc/%.grpc.pb.h: proto/%.proto
	$(PROTOC) -Iproto --grpc_out=src/rpc --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN) $<
src/rpc/%.pb.cc src/rpc/%.pb.h: proto/%.proto
	$(PROTOC) -Iproto --cpp_out=src/rpc $<

test/test_%: test/%.c libpheromone.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

python: $(PROTO_PY) $(GRPC_PROTO_PY)
python/rpc/%_pb2_grpc.py: proto/%.proto
	$(PROTOC) -Iproto --grpc_out=python/rpc --plugin=protoc-gen-grpc=$(GRPC_PYTHON_PLUGIN) $<
python/rpc/%_pb2.py: proto/%.proto
	$(PROTOC) -Iproto --python_out=python/rpc $<

clean:
	rm -f *.a
	rm -f .objs/*.o
	rm -f .objs/rpc/*.o
	rm -f src/rpc/*.h
	rm -f src/rpc/*.cc
	rm -rf python/rpc/*.py
	rm -f $(TESTS)

mrproper: clean

help:
	@echo "Targets:"
	@echo "  all    - Build C library (default)"
	@echo "  python - Build python library"
	@echo "  clean  - Remove all generated files"
	@echo ""
	@echo "Variables:"
	@echo "  CC = $(CC)"
	@echo "  CXX = $(CXX)"
	@echo "  AR = $(AR)"
	@echo "  RANLIB = $(RANLIB)"
	@echo "  PROTOC = $(PROTOC)"
	@echo "  GRPC_CPP_PLUGIN = $(GRPC_CPP_PLUGIN)"
	@echo "  GRPC_PYTHON_PLUGIN = $(GRPC_PYTHON_PLUGIN)"
	@echo "  DEBUG = $(DEBUG)"
	@echo "  WERROR = $(WERROR)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  CXXFLAGS = $(CXXFLAGS)"
	@echo "  LDFLAGS = $(LDFLAGS)"

.PHONY: all python flatcc nanomsg flatbuffers
