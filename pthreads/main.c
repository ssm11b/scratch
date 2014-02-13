
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ring.h"

static int NR_THREADS = 5;
static int LOOPS = 50;

pthread_t threads[100];



#define DELAY_US rand() % 1000
#define DATA_INVALID -1
static char b[4096];

static ring *ioring = (ring *) b;

static volatile long prod_cnt = 0;

static inline uint64_t get_ts(void)
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec;
}

static void producer_func(ring * r)
{
        while (prod_cnt < LOOPS) {
                usleep(DELAY_US);
                if (ring_insert(r, prod_cnt) == 0) {
                        prod_cnt++;
                }
        }

}

static void * consumer_func(void *threadid)
{
        int cnt = 0;
        ring * r = ioring;
        long data;

        printf("[%08lx] %s:%p\n", get_ts(), __func__, threadid);
        

        while (cnt < LOOPS || !ring_empty(r)) {
                data = ring_remove(r);
                if (data == DATA_INVALID) {
                        usleep(DELAY_US);
                } else {
                        printf("[%08lx] %s:%p: %ld\n", get_ts(), __func__, threadid, data);
                        cnt++;
                }
        }

        pthread_exit(NULL);
}

#define ARRAY_SIZE(x)   sizeof(x) / sizeof(*x)
#if 0
static void test_atomics(void)
{
        long v = 100;
        printf("%ld %ld %ld\n", v, atomic_add64(&v, 1), atomic_add64(&v, 5));
        printf("%ld %ld %ld\n", v, atomic_xchg64(&v, 1), atomic_xchg64(&v, 5));
        printf("%ld \n", v);
}
#endif

int main(int argc, char *argv[])
{
        uint64_t i;

        if (argc > 1) LOOPS = atoi(argv[1]);
        if (argc > 2) NR_THREADS = atoi(argv[2]) % 100;


        //test_atomics();
        ring_init(ioring, sizeof(b), DATA_INVALID);
        ring_dump(ioring);

        for (i = 0; i < NR_THREADS; i++) {
                pthread_create(threads+i, NULL, consumer_func, (void*) i+LOOPS);
        }
        producer_func(ioring);
        return 0;
}
