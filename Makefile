CC=g++
CXXFLAGS=-std=c++11 -c $(INCLUDE_DIRS)
INCLUDE_DIRS=-I../rocksdb-5.18.3/include
LDFLAGS=-L../rocksdb-5.18.3 -lrocksdb -lrt -lz -lbz2 -lpthread
SOURCES=cc/power-law.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=power-law

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.cc.o:
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	rm -f *.o
