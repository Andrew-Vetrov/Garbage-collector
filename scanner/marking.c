#include <stdio.h>
#include <sys/mman.h>
#include "../allocator/allocator.h"
#include "../logging/log.h"
#include "marking.h"
#include "stack.h"
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
	DIR* directory;
	struct dirent* inp;
	char* matched_names[BUFSIZ];
	int match_counter = 0;
	int compilation_result;
	int runtime_result;
	directory = opendir("./testing_system");
	char OBJ[] = " ./allocator/allocator.o ./scanner/marking.o ./scanner/stack.o";
	while (1) {
		inp = readdir(directory);
		if (inp == NULL) {
			break;
		}
		if (fnmatch("*.c", inp->d_name, 0) == 0 && strcmp(inp->d_name, "testing_system.c") != 0) {
			char ans[BUFSIZ] = "./testing_system/";
			strcat(ans, inp->d_name);
			matched_names[match_counter] = malloc(strlen(ans) + 1);
			strcpy(matched_names[match_counter], ans);
			match_counter++;
		}
	}
	printf("CONTER %d\n", match_counter);
	for (int i = 0; i < match_counter; i++) {
		int flag = 0;
		char command[BUFSIZ] = "gcc ";
		fprintf(stderr, "\nTesting %s\n", matched_names[i]);

		pid_t pid = fork();
		if (pid == 0) {
			char command[BUFSIZ] = "gcc ";
			strncat(command, matched_names[i], strlen(matched_names[i]) + 1);
			
			if (strcmp(matched_names[i], "./testing_system/lisp_test.c") == 0) {
				flag = 1;
				system("make clean");
				system("make -f Makefile1");
				//strncat(command, " -DLISP=1 ", 9);
			}

			strncat(command, " -L./ -l:lib.a", 22);
			strncat(command, " -o test -w", 13);
			printf("Command %s\n", command);
			compilation_result = system(command);
			if (compilation_result == 256) {
				fprintf(stderr, "\033[1;41mCompilation failed\033[0m\n");
				return 1;
			}
			fprintf(stderr, "\033[1;42mCompiled successfuly\033[0m\n");
			//alarm(10);
			if (flag) {
				system("make clean");
				system("make");
			}
			execl("./test", "./test", (char*)NULL);
		}
		else {
			int status;
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) == 0) {
				fprintf(stderr, "\n\033[1;42mExecuted successfuly\033[0m\n");
				continue;
			}
			fprintf(stderr, "\n\033[1;41mAn error occured while runtime\033[0m\n");

			return 1;
		}
	}
	system("rm test");
	closedir(directory);

	fprintf(stderr, "\nENDED SUCCESSFULLY\n");

void before_main(void) {
    stack = create_stack();
    asm volatile("mov %%rsp, %0" : "=r" (start_rsp_value));
}

void mark(Object object) {
    if (!is_marked(object)) {
        mark_object(object);
        push(stack, get_object_addr(object));
    }
}

void scan(size_t object_addr) {
    Object object;
    if (get_object(object_addr, &object) == INVALID_ADDRESS) {
        return;
    }
    size_t object_start_addr = get_object_addr(object);
    size_t object_end_addr = object_start_addr + get_object_size_by_address(object_start_addr);
    for (size_t inner_object_addr = object_start_addr; inner_object_addr < object_end_addr; inner_object_addr += sizeof(size_t)) {
        Object inner_object;
        if (get_object(*(size_t*)inner_object_addr, &inner_object) == 0) {
            mark(inner_object);
        }
    }
}

void closure() {
    size_t cur_elem;
    while (!is_empty(stack)) {
        cur_elem = pop(stack);
        scan(cur_elem);
    }
}


void push_registers_to_stack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = 0; i < REGISTER_AMOUNT; i++) {
        asm volatile(
            "push %[reg]\n"
            :
        : [reg] "r" (REGISTERS[i])
            );
    }
}

void pop_registers_from_stack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = REGISTER_AMOUNT - 1; i >= 0; i--) {
        asm volatile(
            "pop %[reg]\n"
            :
        : [reg] "r" (REGISTERS[i]));
    }
}

void segment_traverse(size_t segment_start, size_t segment_end) {
    assert(segment_start && segment_end && segment_start < segment_end);
    for (size_t object_addr = segment_start; object_addr < segment_end; object_addr += sizeof(size_t)) {
        Object object;
        if (get_object(*(size_t*)object_addr, &object) == 0) {
            mark(object);
        }
    }
    closure();
}

void collect() {
    full_marking();
    sweep();
}

void full_marking() {
    log(MARK, START);
    segment_traverse(end_rsp_value, start_rsp_value);
    segment_traverse((size_t)&__data_start, (size_t)&edata);
    segment_traverse((size_t)&__bss_start, (size_t)&end);
    log(MARK, OK);
}
