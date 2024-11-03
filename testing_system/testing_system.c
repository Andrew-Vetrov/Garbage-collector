#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fnmatch.h>

int main() {
	DIR* directory;
	struct dirent* inp;
	char* matched_names[BUFSIZ];
	int match_counter = 0;
	int compilation_result;
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
	for (int i = 0; i < match_counter; i++) {
		char command[BUFSIZ] = "gcc ";
		printf("\nTesting %s\n", matched_names[i]);
		strncat(command, matched_names[i], strlen(matched_names[i]) + 1);
		strncat(command, " -o test", 9);
		compilation_result = system(command);
		if (compilation_result == 256) { 
			printf("\033[1;41mCompilation failed\033[0m\n");
			continue;
		}
		printf("\033[1;42mCompiled successfuly\033[0m\n");
		system("valgrind --leak-check=full --leak-resolution=med ./test");
	}
	return 0;
}