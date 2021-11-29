CC=gcc -g
CFLAGS=-c -O2
SOURCES=main.c
#SOURCES=wrapperMain.c
#SOURCES=sep3.c
OBJECTS=$(SOURCES:.c=.o)
INCLUDES=-I/usr/include
#LIBS=-L/home/neeraj/code/lib -lgmp
LIBS=libVajra.a -lgmp -lm
EXE=vajra
all:	$(SOURCES) $(EXE)

$(EXE):		$(OBJECTS)
	$(CC) -o $@ $< $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf $(OBJECTS) $(EXE)
