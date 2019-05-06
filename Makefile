CC=g++
CXXFLAGS=-std=c++11 -c $(INCLUDE_DIRS)
INCLUDE_DIRS=-I../rocksdb-5.18.3/include -Irpclib/include
LDFLAGS=-L../rocksdb-5.18.3 -lrocksdb -Lrpclib/build -lrpc -lrt -lz -lbz2 -lpthread
SOURCES=cc/instance.cc cc/coordinator.cc cc/partitioner.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=build/instance build/coordinator build/partitioner

all: dir $(SOURCES) $(EXECUTABLE)

dir:
	mkdir -p build

build/instance: cc/instance.o
	$(CC) -o $@ $< $(LDFLAGS)

build/coordinator: cc/coordinator.o
	$(CC) -o $@ $< $(LDFLAGS)

build/partitioner: cc/partitioner.o
	$(CC) -o $@ $< $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm -f cc/*.o
