CC=gcc
CFLAGS=-c -O2 -std=gnu18 -Wall -Wextra -Wpedantic 
LDFLAGS= -lm
SOURCES=Image.c Image_Processing.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ImProc

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o ImProc
