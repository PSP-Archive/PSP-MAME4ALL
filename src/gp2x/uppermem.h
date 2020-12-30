#ifndef __GP2XUPPERMEM__
#define __GP2XUPPERMEM__

extern void gp2x_malloc_init(void *pointer);
extern void * gp2x_mallocX(size_t size, const char * func);
extern void gp2x_freeX(void *ptr, const char * func);

extern void * upper_take(int Start, size_t Size);

extern void malloc_debug_init(void);
extern void malloc_debug_status(int flag);

#endif /* __GP2XUPPERMEM__ */
