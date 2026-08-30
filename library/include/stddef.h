#ifndef STDDEF
#define STDDEF

typedef unsigned int size_t;
typedef unsigned int ptrdiff_t;
typedef struct {
    long long __ll;
    long double __ld;
    void *__p;
} max_align_t;

#define NULL ((void *)0)
#define offsetof(st, m) ((size_t)&(((st*)0)->m))

#endif