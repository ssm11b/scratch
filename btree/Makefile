IDIR =./
CC=gcc
CFLAGS=-Wall -g -I$(IDIR) -std=c99
TARGET=btu

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = btree.h btree_file.h debug.h list.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o btree_file.o btree.o test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(ODIR) $(TARGET) 

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(TARGET)
	rm -rf $(ODIR)

