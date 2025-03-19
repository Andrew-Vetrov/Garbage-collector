#include <stdio.h>
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
    asm volatile("mov %%rsp, %0" : "=r" (start_rsp_value));
}


bool is_pointer_valid(size_t object_addr) {
    if (object_addr < START_ALLOCATOR_HEAP || object_addr >= END_ALLOCATOR_HEAP) {              //pointer isn't in heap
        return false;
    }

    size_t block_addr = get_block_addr(object_addr);
    size_t object_addr_in_block = object_addr - block_addr;

    if (object_addr_in_block >= 0 && object_addr_in_block < BLOCK_HEADER_SIZE) {                //pointer to header
        return false;
    }

    size_t object_size = GET_SIZE_WITH_ALIGNMENT(get_object_size_by_address(object_addr));      

    if ((object_addr_in_block - BLOCK_HEADER_SIZE) % object_size != 0) {                        //pointer to wrong position in block
        return false;
    }

    return true;
}

void mark(size_t* elem) {
    //if (elem != START_ALLOCATOR_HEAP && !get_bit_by_address(elem)) {
    printf("%p\n", elem);
    printf("BIT = %d\n", get_bit_by_address(elem));
    if (!get_bit_by_address(elem)) {
        set_bit_by_address(elem, 1);
        push(stack, elem);

        log(MARK, ALIVE);
    }
}

void scan(size_t elem) {
    size_t size = get_object_size_by_address(elem);
    printf("Haha\n");
    for (int i = 0; i < size; i += sizeof(size_t)) {
        if (*((size_t*)(elem + i)) >= START_ALLOCATOR_HEAP && *((size_t*)(elem + i)) < END_ALLOCATOR_HEAP) {
            if (get_object_size_by_address(elem + i) == 0) { 
                continue;
            }
            mark(*((size_t*)(elem + i)));
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
    size_t size;
    stack = create_stack();
    for (size_t i = segment_start; i < segment_end; i += sizeof(size_t)) {
        if (*((size_t*)i) >= START_ALLOCATOR_HEAP && *((size_t*)i) < END_ALLOCATOR_HEAP
            && is_pointer_valid(i)) {
            size = get_object_size_by_address(*((size_t*)i));
            if (size > 0) {
                //printf("Started marking\n");
                mark(*((size_t*)i));
            }
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
    segment_traverse(&__data_start, &edata);
    segment_traverse(&__bss_start, &end);

    log(MARK, OK);
}
