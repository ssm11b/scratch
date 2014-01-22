
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#define NR_THREADS 5
pthread_t threads[NR_THREADS];

#define LOOPS 5
static volatile int prod_cnt = 0;

static inline uint64_t get_ts(void)
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec;
}

static void producer_func(void)
{
        while (prod_cnt < LOOPS) {
                usleep(2000);
                printf("[%lld]%s: %d\n", get_ts(),  __func__, prod_cnt);
                prod_cnt++;
        }

}

static void* consumer_func(void *threadid)
{
        int cnt = 0;
        printf("[%lld] %s:%p\n", get_ts(), __func__, threadid);

        while (cnt < LOOPS) {
                if (cnt == prod_cnt) {
                        usleep(1000);
                } else {
                        printf("[%lld] %s:%p: %d\n", get_ts(), __func__, threadid, prod_cnt);
                        cnt = prod_cnt;
                }
        }

        pthread_exit(NULL);
}

#define ARRAY_SIZE(x)   sizeof(x) / sizeof(*x)

int main(int argc, char *argv[])
{
        int rc;
        uint64_t i;

        printf("ARRAY_SIZE %ld\n", ARRAY_SIZE(threads));

        for (i = 0; i< NR_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, consumer_func, (void*) i);
        }
        producer_func();
        return 0;
}
