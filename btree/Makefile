IDIR =./
CC=gcc
CFLAGS=-I$(IDIR)
TARGET=btree-util

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = btree.h btree_file.h debug.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o btree_file.o
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

