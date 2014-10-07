IDIR=.
CC=g++
CFLAGS=-Wall -g -I$(IDIR) 
TARGET=ring_unit

ODIR=./obj
LIBS=-lpthread

_DEPS = $(IDIR)/*.h # some include file
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = ring_unit.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(ODIR) $(TARGET) 

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(TARGET)
	rm -rf $(ODIR)

