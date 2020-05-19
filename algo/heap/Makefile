IDIR=.
CC=gcc
CFLAGS=-Wall -I$(IDIR) -std=c11 #-Wno-delete-non-virtual-dtor #-pedantic -Wno-c++11-extensions
TARGET=heap

ODIR=./obj
LIBS=

_DEPS = # $(IDIR)/*.h # some include file
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = heap.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(ODIR) $(TARGET) 

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(TARGET)
	rm -rf $(ODIR)

