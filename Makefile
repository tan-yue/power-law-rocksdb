CC=g++
CXXFLAGS=-std=c++11 -c $(INCLUDE_DIRS)
INCLUDE_DIRS=-Irocksdb-5.18.3/include -Irpclib/include
LDFLAGS=-Lrocksdb-5.18.3 -lrocksdb -lrt -lz -lbz2
RPC_LDFLAGS=-Lrpclib/build -lrpc -lpthread
SOURCES=cc/instance.cc cc/coordinator.cc cc/partitioner.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=build/instance build/coordinator build/partitioner

all: dir $(SOURCES) $(EXECUTABLE)

debug: CXXFLAGS += -DDEBUG -g
debug: $(EXECUTABLE)

dir:
	mkdir -p build

build/instance: cc/instance.o
	$(CC) -o $@ $< $(LDFLAGS) $(RPC_LDFLAGS)

build/coordinator: cc/coordinator.o
	$(CC) -o $@ $< $(RPC_LDFLAGS)

build/partitioner: cc/partitioner.o
	$(CC) -o $@ $< $(RPC_LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm -f cc/*.o
