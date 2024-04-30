SRC_DIR := src
INC_DIR := include

CFLAGS := -Wall -g -I$(INC_DIR)
CPPFLAGS := $(CFLAGS) -std=c++17
LDFLAGS := -lm -lGLEW -lGL -lglfw -ldl

CPP_SRCS := $(wildcard $(SRC_DIR)/**/**/*.cpp $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
CPP_OBJS := $(CPP_SRCS:.cpp=.o)

OBJS := $(C_OBJS) $(CPP_OBJS)

TARGET := main

all: build $(TARGET) post_build

$(TARGET): $(OBJS)
	g++ -o $@ $^ $(LDFLAGS)
	@chmod a+rx $(TARGET)

%.o: %.cpp
	g++ $(CPPFLAGS) -c $< -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

.PHONY: build post_build clean_objects clean

build:
	@echo -e "  ┌───────────"
	@echo -e "  │ Starting build..."
	@echo -e "  └──"

post_build: $(TARGET)
	@echo -e "  ┌───────────"
	@echo -e "  │ Finished compile jobs, cleaning object files..."
	@echo -e "  └──"
	@#make clean_objects
	@echo -e "\n  ┌───────────"
	@echo -e "  │ Finished all Makefile rules. Binary should be \"$(TARGET)\""
	@echo -e "  └──\n"

clean_objects:
	find $(SRC_DIR) -type f -name '*.o' -exec rm {} +

clean: clean_objects
	@echo -e "  ┌───────────"
	@echo -e "  │ Cleaning object and binary files..."
	@echo -e "  └──"
	@rm -vf $(TARGET)
