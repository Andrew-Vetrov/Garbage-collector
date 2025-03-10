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
	return 0;
}
