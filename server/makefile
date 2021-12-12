TARGET=server
SRC = $(wildcard *.cpp *.c)
OBJ = $(patsubst %.cpp *.c,%.o,$(SRC))
DEFS =
CFLAGS = -g
CC =g++
LIBS =  -lpthread
$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $^ $(LIBS)
.PHONY:
clean:
	rm -rf *.o $(TARGET)