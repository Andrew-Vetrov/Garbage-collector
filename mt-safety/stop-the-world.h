__attribute__((constructor)) void __init_stop_the_world();

__attribute__((destructor)) void __destroy_stop_the_world();

/* you have to call start and stop the world from the same thread! */
void stop_the_world();

void start_the_world();