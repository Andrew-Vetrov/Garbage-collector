LIB_NAME = lib.a

CC = gcc

CFLAGS = -w

SRC = allocator/allocator.c scanner/marking.c scanner/stack.c mt-safety/mt-safety.c

OBJ = $(SRC:.c=.o)

BUILD_DIR = build

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	ar rcs $@ $^

%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

clean:
	rm $(OBJ) $(LIB_NAME)
