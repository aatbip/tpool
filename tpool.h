#ifndef tpool_h
#define tpool_h

typedef struct _tpool tpool;

tpool *tpool_create(int thread_count, int buffer_size);
int tpool_add(tpool *tp, void (*func)(void *), void *arg);
void tpool_wait(tpool *tp);
int tpool_shutdown(tpool *tp);
int tpool_destroy(tpool *tp);
int tpool_count_working_threads(tpool *tp);

#endif // !tpool_h
