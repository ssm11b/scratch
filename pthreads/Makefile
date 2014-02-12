IDIR =../include/
CC=gcc
CFLAGS=-Wall -g -I$(IDIR) 
TARGET=test

ODIR=obj
#LDIR =../lib

LIBS=-lpthread

_DEPS = # some include file
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o 
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

