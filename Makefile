LIB_NAME = lib.a

CC = gcc

CFLAGS = -w

SRC = ./allocator/allocator.c ./scanner/marking.c ./scanner/stack.c

OBJ = $(SRC:.c=.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	ar rcs $@ $^

%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

clean:
	rm $(OBJ) $(LIB_NAME)
