#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

int main() {
	DIR* directory;
	struct dirent* inp;
	char command[BUFSIZ] = "gcc ";
	directory = opendir(".");

	while (1) {
		inp = readdir(directory);
		if (inp == NULL) {
			break;
		}
		if (strcmp(inp->d_name, "testing_system.c") == 0
			|| strcmp(inp->d_name, ".") == 0
			|| strcmp(inp->d_name, "..") == 0
			|| strchr(inp -> d_name, '.') == NULL) {
			continue;
		}
		printf("Testing %s\n", inp->d_name);
		strncat(command, inp->d_name, strlen(inp->d_name) + 1);
		strncat(command, " -o test", 9);
		system(command);
		system("./test");
		break;
	}
	return 0;
}