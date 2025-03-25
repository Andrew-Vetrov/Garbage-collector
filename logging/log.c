#include "log.h"

FILE* log_file = NULL;
size_t prev_object_size = -1, last_object_size = 0, memory_limit = 0, memory_used = 0, object_count = 0, marked_objects = 0, m_size = 0;
clock_t start_time = 0, end_time = 0;
int mark_stage_count = 0, sweep_stage_count = 0, new_objects_count = 0;

bool extra_log = false;

char log_file_name[25];

char log_buffer[BUFFER_SIZE];
char* buf_pos = log_buffer;

static double log_time() {
	end_time = clock();
	return ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
}

static void printf_in_file() {
	if (log_file) {
		buf_pos[0] = 0;
		fprintf(log_file, "%s", log_buffer);
		buf_pos = log_buffer;
	}

	else {
		fprintf(stderr, "log_file is NULL.\n");
	}
}

static void add_log_line(const char* format, ...) {
	if (new_objects_count != 0 && extra_log == false) {
		buf_pos += sprintf(buf_pos, "[%010.6lf] OK. New objects with total size %lu bytes (count: %d, size of one object: %lu).\n\tUsed space: %lu/%lu bytes.\n\tNumber of objects: %lu.\n\tFree space: %lu/%lu bytes.\n", log_time(), last_object_size * (size_t)new_objects_count, new_objects_count, last_object_size, memory_used, memory_limit, object_count, memory_limit - memory_used, memory_limit);
		
		if (log_buffer + BUFFER_SIZE - buf_pos <= BUFFER_DEAD_NUMBER) {
			printf_in_file();
		}
		
		prev_object_size = -1;
		new_objects_count = 0;
	}

	va_list args;
	va_start(args, format);

	buf_pos += vsprintf(buf_pos, format, args);

	if (extra_log == true) {
		extra_log = false;
	}
	
	if (log_buffer + BUFFER_SIZE - buf_pos <= BUFFER_DEAD_NUMBER) {
		printf_in_file();
	}

#ifdef DEBUG
	vprintf(format, args);
#endif

	va_end(args);
}

static void log_new_object() {
	if (last_object_size != prev_object_size && prev_object_size != -1) {
		extra_log = true;
		add_log_line("[%010.6lf] OK. New objects with total size %lu bytes (count: %d, size of one object: %lu).\n\tUsed space: %lu/%lu bytes.\n\tNumber of objects: %lu.\n\tFree space: %lu/%lu bytes.\n", log_time(), last_object_size * (size_t)new_objects_count, new_objects_count, last_object_size, memory_used, memory_limit, object_count, memory_limit - memory_used, memory_limit);
		new_objects_count = 0;
	}

	new_objects_count++;
	prev_object_size = last_object_size;
}

void set_memory_limit(size_t heap_size, size_t percent) {
	memory_limit = heap_size * percent / 100;
#ifdef LOG
	add_log_line("[%010.6lf] User memory limit set: %lu bytes.\n", log_time(), memory_limit);
#endif
}

void log(log_t type, log_t result) {
	switch (type) {
#ifdef LOG
	case INIT_ALLOCATOR:
		switch (result) {
			FILE* build_count;

		case START:
			build_count = fopen("build_count.log", "a+");
			if (!build_count) {
				fprintf(stderr, "Error opening the file \"build_count.log\".\n");
			}

			if (fseek(build_count, 0, SEEK_END)) {
				fprintf(stderr, "Error: fseek.\n");
				fclose(build_count);
			}

			long file_size = ftell(build_count);
			if (file_size == -1L) {
				fprintf(stderr, "Error: ftell.\n");
				fclose(build_count);
			}

			if (!file_size) {
				fprintf(build_count, "1\n");
				log_file_name[0] = '1';
				log_file_name[1] = '.', log_file_name[2] = 'l', log_file_name[3] = 'o', log_file_name[4] = 'g';
				log_file_name[5] = 0;

				fclose(build_count);
			}

			else {
				int last_build_number, indx_in_name = 0, string_len = 0;
				if (fseek(build_count, 0, SEEK_SET)) {
					fprintf(stderr, "Error: fseek.\n");
					fclose(last_build_number);
				}

				fscanf(build_count, "%d", &last_build_number);
				fclose(build_count);

				build_count = fopen("build_count.log", "w");
				if (!build_count) {
					fprintf(stderr, "Error opening the file \"build_count.log\".\n");
				}

				fprintf(build_count, "%d\n", ++last_build_number);
				fclose(build_count);

				int last_build_number_copy = last_build_number;

				while (last_build_number_copy) {
					last_build_number_copy /= 10;
					string_len++;
				}

				while (last_build_number) {
					log_file_name[string_len - 1 - indx_in_name] = (last_build_number % 10) + '0';
					last_build_number /= 10;
					indx_in_name++;
				}

				log_file_name[string_len++] = '.', log_file_name[string_len++] = 'l', log_file_name[string_len++] = 'o', log_file_name[string_len++] = 'g';
				log_file_name[string_len++] = 0;
			}

			log_file = fopen(log_file_name, "a+");
			if (!log_file) {
				fprintf(stderr, "Error opening log_file.\n");
			}
			
			add_log_line("[000.000000] Start of allocator initialization.\n");
			start_time = clock();

			break;

		case B_ERROR:
			add_log_line("[%010.6lf] Can't allocate BIG_allocator's heap!\n", log_time());

			break;

		case ERROR:
			add_log_line("[%010.6lf] Can't allocate allocator's heap!\n", log_time());

			break;

		case OK:
			add_log_line("[%010.6lf] Allocator initialized.\n", log_time());

			break;
		}

		break;

	case DESTROY_ALLOCATOR:
		switch (result) {
		case B_ERROR:
			add_log_line("[%010.6lf] Can't unmap BIG_heap!\n", log_time());

			break;
		
		case ERROR:
			add_log_line("[%010.6lf] Can't unmap heap!\n", log_time());

			break;

		case OK:
			add_log_line("[%010.6lf] Allocator destroyed.\n", log_time());

			if (buf_pos != log_file) {
				printf_in_file();
			}
			printf("DESTROY\n");

			break;
		}

		break;
#endif

	case ALLOCATE_NEW_OBJECT:
		switch (result) {
#ifdef LOG
		case ERROR:
			add_log_line("[%010.6lf] Denied: size of object is too large.\n", log_time());

			break;

		case B_HEAP_ERROR:
			add_log_line("[%010.6lf] No memory in BIG heap!\n", log_time());

			break;

		case HEAP_ERROR:
			add_log_line("[%010.6lf] No memory in small heap!\n", log_time());

			break;
#endif

		case OK:
			object_count++;
			memory_used += last_object_size;
#ifdef LOG
#ifndef FL
			log_new_object();
#else
			add_log_line("[%010.6lf] OK. New object of size %lu bytes.\n\tUsed space: %lu/%lu bytes.\n\tNumber of objects: %lu.\n\tFree space: %lu/%lu bytes.\n", log_time(), last_object_size, memory_used, memory_limit, object_count, memory_limit - memory_used, memory_limit);
#endif
#endif
			break;
		}

		break;

#ifdef LOG
	case MARK:
		switch (result) {
		case START:
			mark_stage_count++;
			add_log_line("[%010.6lf] Mark stage %d started.\n", log_time(), mark_stage_count);
			marked_objects = 0;

			break;

		case ALIVE:
			marked_objects++;
			
			break;

		case OK:
			add_log_line("[%010.6lf] Mark stage %d completed. Objects collected: %lu.\n\t%lu bytes of memory were freed.\n", log_time(), mark_stage_count, object_count - marked_objects, memory_used - m_size);
			object_count = marked_objects;
			memory_used = m_size;
			m_size = 0;

			break;
		}

		break;

	case SWEEP:
		switch (result) {
		case START:
			sweep_stage_count++;
			add_log_line("[%010.6lf] Sweep stage %d started.\n", log_time(), sweep_stage_count);

			break;

		case OK:
			add_log_line("[%010.6lf] Sweep stage %d completed.\n", log_time(), sweep_stage_count);

			break;
		}

		break;

	case OTHER:
		switch (result) {
		case O_EMPTY_BLOCK:
			add_log_line("[%010.6lf] No empty blocks in garbage collector!\n", log_time());

			break;

		case O_HEADER:
			add_log_line("[%010.6lf] No empty headers in garbage collector!\n", log_time());

			break;
		}

		break;
#endif
	}
}

log_t check_the_space(size_t object_size) {
	last_object_size = object_size;
	
	if (memory_used + object_size > memory_limit) {
#ifdef LOG
		add_log_line("[%010.6lf] Denied: memory limit exceeded. Free space: %lu/%lu bytes.\n", log_time(), memory_limit - memory_used, memory_limit);
#endif
		return ERROR;
	}

	return OK;
}

void log_mark_alive(size_t object_size) {
	m_size += object_size;
	log(MARK, ALIVE);
}