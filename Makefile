LIB_NAME = libgc.a

CC = gcc

CFLAGS = -w

SRC_DIRS = allocator marker logging mt-safety sweeper memops
SRC = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) gc.c

OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

BUILD_DIR = build

all: $(LIB_NAME)

$(LIB_NAME): $(OBJ)
	ar rcs ${BUILD_DIR}/$@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

lisp_test: CFLAGS += -DHEAP_SIZE=65536
lisp_test: all

run_tests: all
	gcc testing_system/testing_system.c -o ${BUILD_DIR}/run_tests -DDEBUG && ./${BUILD_DIR}/run_tests