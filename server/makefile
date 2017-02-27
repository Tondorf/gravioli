CC=g++
CFLAGS=-c -Wall -Wextra -std=c++11
LDFLAGS=-levent -lprotobuf -lpthread
SOURCES=main.cpp server.cpp client.cpp logger.cpp world.cpp gravioli.pb.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: proto $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

proto:
	protoc -I=.. ../gravioli.proto --cpp_out=.

clean:
	rm *.o $(EXECUTABLE)
