CC=g++
CXXFLAGS=-std=c++11 -c $(INCLUDE_DIRS)
INCLUDE_DIRS=-I../rocksdb-5.18.3/include -Irpclib/include
LDFLAGS=-L../rocksdb-5.18.3 -lrocksdb -Lrpclib/build -lrpc -lrt -lz -lbz2 -lpthread
SOURCES=cc/*.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=build/instance build/coordinator build/partitioner

all: dir $(SOURCES) $(EXECUTABLE)

dir:
	mkdir -p build

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm -f cc/*.o
