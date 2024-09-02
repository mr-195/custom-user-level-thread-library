#ifndef FOOTHREAD_H
#define FOOTHREAD_H

#include <semaphore.h>

#define FOOTHREAD_THREADS_MAX 256
#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152 
#define FOOTHREAD_JOINABLE 1
#define FOOTHREAD_DETACHED 0

#define FOOTHREAD_ATTR_INITIALIZER { FOOTHREAD_DETACHED, FOOTHREAD_DEFAULT_STACK_SIZE }

typedef struct {
    pid_t tid;
    void *stack;
    int joinable;
} foothread_t;

typedef struct {
    int join_type;
    size_t stack_size;
} foothread_attr_t;

int foothread_create(foothread_t *, foothread_attr_t *, int (*)(void *), void *);
int foothread_attr_setjointype(foothread_attr_t *, int);
int foothread_attr_setstacksize(foothread_attr_t *, size_t);
void foothread_exit();

typedef struct {
    pid_t tid;
    sem_t sem;
    int locked;
} foothread_mutex_t;

void foothread_mutex_init(foothread_mutex_t *);
void foothread_mutex_lock(foothread_mutex_t *);
void foothread_mutex_unlock(foothread_mutex_t *);
void foothread_mutex_destroy(foothread_mutex_t *);

typedef struct {
    int count;
    int threshold;
    sem_t waiting_sem;
    foothread_mutex_t mutex;
} foothread_barrier_t;

void foothread_barrier_init(foothread_barrier_t *, int);
void foothread_barrier_wait(foothread_barrier_t *);
void foothread_barrier_destroy(foothread_barrier_t *);

#endif /* FOOTHREAD_H */
