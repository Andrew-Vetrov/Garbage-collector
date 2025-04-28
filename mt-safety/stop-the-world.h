__attribute__((constructor)) void __init_stop_the_world();

void prepare_thread_to_stop();

/* you have to call start and stop the world from the same thread! */
void stop_the_world();

void start_the_world();