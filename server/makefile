CC=g++
CFLAGS=-c -Wall -Wextra -std=c++11
LDFLAGS=-levent -lprotobuf -lpthread
SOURCES=main.cpp server.cpp client.cpp logger.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXECUTABLE)
