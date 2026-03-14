#ifndef tpool_h
#define tpool_h

typedef struct _tpool tpool;

tpool *tpool_create(int thread_count);
int tpool_add(tpool *tp, void *(*func)(void *), void *arg);

#endif // !tpool_h
