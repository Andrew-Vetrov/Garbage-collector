#include "log.h"

FILE* log_file = NULL;
size_t last_object_size = 0, memory_limit = 0, memory_used = 0, object_count = 0, marked_objects = 0;
clock_t start_time = 0, end_time = 0;
char log_file_name[20];

static double log_time() {
	end_time = clock();
	return ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
}

static void add_log_line(const FILE* file, const char* format, ...) {
	va_list args;
	va_start(args, format);

	vfprintf(file, format, args);

#ifdef DEBUG
	vprintf(format, args);
#endif

	va_end(args);
}

void set_memory_limit(size_t heap_size, size_t percent) {
	memory_limit = heap_size * percent / 100;
	add_log_line(log_file, "[%09.4lf] User memory limit set: %lu bytes.\n", log_time(), memory_limit);
}

void log(log_t type, log_t result) {
	switch (type) {
		case INIT_ALLOCATOR:
			switch (result) {
				FILE* build_count;

				case START:
					build_count = fopen("build_count.log", "a+");
					if (!build_count) {
						perror("Error opening the file \"build_count.log\".\n");
					}

					if (fseek(build_count, 0, SEEK_END)) {
						perror("Error: fseek.\n");
						fclose(build_count);
					}

					long file_size = ftell(build_count);
					if (file_size == -1L) {
						perror("Error: ftell.\n");
						fclose(build_count);
					}

					if (!file_size) {
						fprintf(build_count, "1");
						log_file_name[0] = '1';
						log_file_name[1] = '.', log_file_name[2] = 'l', log_file_name[3] = 'o', log_file_name[4] = 'g';
						log_file_name[5] = 0;
					}

					else {
						int last_build_number, indx_in_name = 0, number_len = 0;
						fscanf(build_count, "%d", &last_build_number);
						fclose(build_count);

						build_count = fopen("build_count.log", "w");
						if (!build_count) {
							perror("Error opening the file \"build_count.log\".\n");
						}

						fprintf(build_count, "%d", ++last_build_number);
						fclose(build_count);

						int last_build_number_copy = last_build_number;

						while (last_build_number_copy) {
							last_build_number_copy /= 10;
							number_len++;
						}

						while (last_build_number) {
							log_file_name[number_len - 1 - indx_in_name] = last_build_number % 10;
							last_build_number /= 10;
							indx_in_name++;
						}

						log_file_name[number_len++] = '.', log_file_name[number_len++] = 'l', log_file_name[number_len++] = 'o', log_file_name[number_len++] = 'g';
						log_file_name[number_len++] = 0;
					}

					log_file = fopen(log_file_name, "a");
					if (!log_file) {
						perror("Error opening log_file.\n");
					}

					add_log_line(log_file, "[0000.0000] Start of allocator initialization.\n");
					start_time = clock();

					break;

				case ERROR:
					add_log_line(log_file, "[%09.4lf] Error initializing allocator: mmap() function returned MAP_FAILED.\n", log_time());
					
					break;

				case OK:
					add_log_line(log_file, "[%09.4lf] Allocator initialized.\n", log_time());

					break;
			}

			break;

		case DESTROY_ALLOCATOR:
			switch (result) {
				case ERROR:
					add_log_line(log_file, "[%09.4lf] Error destroying allocator: munmap() function returned -1.\n", log_time());

					break;

				case OK:
					add_log_line(log_file, "[%09.4lf] Allocator destroyed.\n", log_time());

					break;
			}

			break;

		case ALLOCATE_NEW_OBJECT:
			switch (result) {
				case ERROR:
					add_log_line(log_file, "[%09.4lf] Denied: size of object is too large.\n", log_time());

					break;

				case OK:
					add_log_line(log_file, "[%09.4lf] OK. New object of size %lu bytes.\n\tUsed space: %lu/%lu bytes.\n\tNumber of objects: %lu.\n\tFree space: %lu/%lu bytes.\n", log_time(), last_object_size, memory_used += last_object_size, memory_limit, ++object_count, memory_limit - memory_used, memory_limit);

					break;
			}

		case MARK:
			switch (result) {
				case START:
					add_log_line(log_file, "[%09.4lf] The Mark stage started.\n", log_time());
					marked_objects = 0;

					break;

				case OK:
					add_log_line(log_file, "[%09.4lf] A new object marked. Total marked objects: %lu.\n", log_time(), ++marked_objects);

					break;
			}
	}
}

int check_the_space(size_t object_size) {
	last_object_size = object_size;
	add_log_line(log_file, "[%09.4lf] Request for space for a new object of %lu bytes in size.\n", log_time(), object_size);

	if (memory_used + object_size > memory_limit) {
		add_log_line(log_file, "[%09.4lf] Denied: memory limit exceeded. Free space: %lu/%lu bytes.\n", log_time(), memory_limit - memory_used, memory_limit);
		
		return -1;
	}

	return 0;
}