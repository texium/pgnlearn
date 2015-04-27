CPP = g++
CPPFLAGS = -DNDEBUG -Wall -O3
LINKFLAGS = -static-libgcc -static-libstdc++

BIN = pgnlearn.exe

OBJ = bitboards.o  \
      book.o       \
      eval.o       \
      main.o       \
      moves.o      \
      notation.o   \
      position.o   \
      regression.o \
      utils.o

LINKOBJ = $(OBJ)

.PHONY: all clean

all: $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKFLAGS) $(LINKOBJ) -o $(BIN)

.o: .cpp 
	$(CPP) $(CPPFLAGS) -c $< -o $@
