CC=gcc
CFLAGS=-g -Wall -Wextra -Iinclude -fsanitize=address
LIBS=-lm
TARGET=cam_detect

SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

_DEPS = bitmap.h cam_detect.h
_OBJS = main.o bitmap.o cam_detect.o

DEPS = $(patsubst %,$(INC_DIR)/%,$(_DEPS))
OBJS = $(patsubst %,$(OBJ_DIR)/%,$(_OBJS))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(CFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) 
