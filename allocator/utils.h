#include <assert.h>
#include <stdlib.h>

#ifdef NDEBUG
#define my_assert(expr) ((void)expr)
#else
#define my_assert assert
#endif

extern size_t end_rsp_value;

void show_bitmap(size_t object_addr);