CPP = nsh.cpp
HEADERS = nsh.h
CC = g++
CFLAGS = -Wall

nsh: $(CPP) $(HEADERS)
	$(CC) $(CFLAGS) $(CPP) -o nsh
clean:
	rm -i nsh
