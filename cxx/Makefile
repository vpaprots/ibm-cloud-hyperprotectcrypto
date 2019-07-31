HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
SYSTEM ?= $(HOST_SYSTEM)
CXX = g++
CXXFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11 -I.
DEPFLAGS = -MMD -MP -MF build/$*.d
LDFLAGS += -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc`\
           -pthread\
           -ldl -lcurl -lcrypto

ifeq ($(SYSTEM),Darwin)
LDFLAGS += -lgrpc++_reflection\
           -L/usr/local/opt/openssl/lib
CXXFLAGS += -I/usr/local/opt/openssl/include
else
    #...
endif

SOURCES = examples/main.cc examples/signature.cc \
	util/credentials.cc \
	$(wildcard grpc/*.cc)
OBJECTS = $(patsubst %.cc,build/%.o,$(SOURCES))
DEPS = $(patsubst %.cc,build/%.d,$(SOURCES))
all:	test

build/cxxexamples: $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) $(DEPFLAGS) -o $@
	
build/%.o:	%.cc
	mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $(DEPFLAGS) -o $@ $<

.PHONY:	test
test:	build/cxxexamples
	build/cxxexamples

clean:
	rm -rf build

include $(wildcard $(DEPS))
