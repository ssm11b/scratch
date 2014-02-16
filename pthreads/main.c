
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include "ring.h"

static int NR_THREADS = 5;
static long LOOPS = 50;

#define MAX_THREADS 256
pthread_t threads[MAX_THREADS + 1];
void *  status[MAX_THREADS + 1];

#define DELAY_US rand() % 10
#define DATA_INVALID -1

static char b[4096];
static ring *ioring = (ring *) b;

char * prod_set;
char * cons_set;
static long prod_cnt = 0;
static long cons_cnt = 0;

static inline uint64_t get_ts(void)
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec;
}

static void producer_func(ring * r)
{
        while (prod_cnt < LOOPS) {
                usleep(1);
                if (ring_insert(r, prod_cnt) == 0) {
                        prod_set[prod_cnt]++;
                        assert(prod_set[prod_cnt] == 1);
                        prod_cnt++;
                }
        }

}

static void * consumer_func(void *threadid)
{
        ring * r = ioring;
        long data;

//        printf("[%08lx] %s:%p\n", get_ts(), __func__, threadid);

        while (cons_cnt < LOOPS || !ring_empty(r)) {
                data = ring_remove(r);
                if (data == DATA_INVALID) {
                        usleep(1);
                } else {
//                        printf("[%08lx] %s:%p: %ld\n", get_ts(), __func__, threadid, data);
                        atomic_inc64(&cons_cnt);
                        assert(cons_set[data] == 0);
                        cons_set[data]++;
                        assert(data <= prod_cnt);
                        assert(cons_set[data] == 1);

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
        int rc;

        if (argc > 1) LOOPS = atoi(argv[1]);
        if (argc > 2) NR_THREADS = atoi(argv[2]) % MAX_THREADS;

        if (NR_THREADS == 0) 
                NR_THREADS = 1;
        if (LOOPS == 0)
                LOOPS = 50;

        prod_set = malloc(LOOPS);
        memset(prod_set, 0, LOOPS);
        cons_set = malloc(LOOPS);
        memset(cons_set, 0, LOOPS);

        //test_atomics();
        ring_init(ioring, sizeof(b), DATA_INVALID);
        ring_dump(ioring);

        for (i = 0; i < NR_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, consumer_func, (void*) i+LOOPS);
                if (rc)
                {
                        printf("failed to create thread %ld", i);
                        exit(1);
                }
        }
        producer_func(ioring);

        for (i = 0; i < NR_THREADS; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld", i);
                        exit(1);
                }

        }

        printf("prod_cnt %ld cons_cnt %ld\n", prod_cnt, cons_cnt);
        for (i = 0; i < LOOPS; i++)
        {
                assert(prod_set[i] == cons_set[i]);
        }
        return 0;
}
