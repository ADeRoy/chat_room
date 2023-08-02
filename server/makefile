TARGET=server
SRC = $(wildcard $(shell pwd)/public/src/*.cpp $(shell pwd)/src/*.cpp)
OBJ = $(patsubst %.cpp *.c,%.o,$(SRC))
DEFS =
CFLAGS = -g -I $(shell pwd)/public/include -I $(shell pwd)/include
CC =g++
LIBS =  -lpthread
$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $^ $(LIBS)
.PHONY:
clean:
	rm -rf *.o $(TARGET)