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
	directory = opendir(".");

	while (1) {
		inp = readdir(directory);
		if (inp == NULL) {
			break;
		}
		if (fnmatch("*.c", inp->d_name, 0) == 0 && strcmp(inp->d_name, "testing_system.c") != 0) {
			matched_names[match_counter++] = inp->d_name;
		}
	}
	system("gcc -c ../scanner/segment_traverse.c ../allocator/allocator.c -w");
	system("ar r testlib.a *.o");
	for (int i = 0; i < match_counter; i++) {
		char command[BUFSIZ] = "gcc ";
		printf("\nTesting %s\n", matched_names[i]);
		pid_t pid = fork();
		if (pid == 0) {
			char command[BUFSIZ] = "gcc ";
			strncat(command, matched_names[i], strlen(matched_names[i]) + 1);
			strncat(command, " -o test *.o -L./ -l:testlib.a", 123);
			compilation_result = system(command);
			if (compilation_result == 256) {
				printf("\033[1;41mCompilation failed\033[0m\n");
				continue;
			}
			printf("\033[1;42mCompiled successfuly\033[0m\n");
			alarm(3);
			execl("./test", "./test", (char*)NULL);
		}
		else {
			int status;
			waitpid(pid, &status, 0);
			if (WIFSIGNALED(status)) {
				printf("\033[1;43mTime limit exceed\033[0m\n");
				continue;
			}
			if (WEXITSTATUS(status) == 0) {
				printf("\033[1;42mExecuted successfuly\033[0m\n");
				continue;
			}
			printf("\033[1;41mAn error occured while runtime\033[0m\n");
		}
	}
	system("rm test");
	system("rm *.o");
	system("rm testlib.a");
	closedir(directory);
	return 0;
}