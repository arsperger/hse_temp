TARGET = client
LIBS =
CC = g++
CFLAGS = -g -Wall
STD = c++14

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.hpp)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) -std=$(STD) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

rebuild: clean all
