TARGET=server
BUILD_DIR = build/obj
SRC = $(wildcard $(shell pwd)/public/src/*.cpp $(shell pwd)/src/*.cpp)
OBJ = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SRC)))
DEFS =
CFLAGS = -g -I $(shell pwd)/public/include -I $(shell pwd)/include
CC = g++
LIBS = -lpthread

# 创建必要的目录
$(shell mkdir -p $(BUILD_DIR))

# 设置 vpath 来查找源文件
vpath %.cpp $(shell pwd)/public/src:$(shell pwd)/src

$(BUILD_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(DEFS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)