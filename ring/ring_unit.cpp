/*
 * Unit tests of the ring producer/consumer FIFO.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#ifdef OSX
#include <sched.h>
#define pthread_yield sched_yield
#endif

#define ASSERT assert

#define RING_CONS
#define RING_PROD
#define RING_TRACE
#include "./ring.h"

static const long NR_PROD_THREADS = 2;
static const long NR_CONS_THREADS = 16;
static const long NR_THREADS = NR_PROD_THREADS + NR_CONS_THREADS;
static const long LOOPS = (64)*(1024)*(1024);
static long nr_threads = NR_CONS_THREADS;
#define MIN(a, b) (a < b ? a : b)

#define MAX_THREADS NR_CONS_THREADS + NR_PROD_THREADS
pthread_t threads[MAX_THREADS];
void *  status[MAX_THREADS];

#define DELAY_US rand() % 10
#define DATA_INVALID -1

#define NR_MAX_ENTS     4096
#define NR_ENTS         256
static char b[RING_ALLOC_SIZE(NR_MAX_ENTS)];
static ring *ioring = (ring *) b;

static atomic_u64 prod_cnt = {0};
static atomic_u64 cons_cnt = {0};
static atomic_u64 queued = {0};
static atomic_u64 produced_rand = {0};
static atomic_u64 consumed_rand = {0};

static atomic_u64 prod_share[NR_PROD_THREADS];
static atomic_u64 prod_priv[NR_PROD_THREADS];
static atomic_u64 cons_priv[NR_CONS_THREADS];

void stats_init()
{
        atomic_u64_set(&prod_cnt, 0);
        atomic_u64_set(&cons_cnt, 0);
        atomic_u64_set(&queued, 0);
        atomic_u64_set(&produced_rand, 0);
        atomic_u64_set(&consumed_rand, 0);

        memset(prod_share, 0, sizeof(*prod_share));
        memset(prod_priv, 0, sizeof(*prod_priv));
        memset(cons_priv, 0, sizeof(*cons_priv));
        memset(b, 0xa5, sizeof(b));
}

static void * producer_func(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        size_t          prod_idx = (thr - threads) - nr_threads;
        ring *          r = ioring;
        atomic_u64 *   priv = &prod_priv[prod_idx];
        long            data;
        size_t          dnar;

        ASSERT(prod_idx < NR_THREADS);
        printf("prod %p %ld\n", threadid, thr-threads);
        while (atomic_u64_get(&prod_cnt) < LOOPS) {
                bool yield = false;
		atomic_inc64(&queued);
		if (atomic_u64_get(&queued) >= NR_ENTS-NR_THREADS) {
			atomic_dec64(&queued);
			pthread_yield();
			continue;
		}
                dnar = rand();
                data = (prod_idx << 56) | dnar;
                if (atomic_add64(&prod_cnt, 1) <= LOOPS) {
                        if (ring_insert(r, data) == 0) {
                                atomic_inc64(priv);
                                atomic_add64(&produced_rand, dnar);
                        } else {
                                yield = true;
                        }
                } else {
                        yield = true;
                }

                if (yield) {
                        atomic_dec64(&prod_cnt);
                        pthread_yield();
                } else {
                        if ((atomic_u64_get(&prod_cnt) & 0xfffff) == 0) {
                                printf("produced %lx of %lx\n",
                                        atomic_u64_get(&prod_cnt), LOOPS);
                        }
                }
        }
        printf("prod %ld produced %ld\n", thr-threads, atomic_u64_get(priv));
        pthread_exit(NULL);
}

static void * consumer_func(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        size_t          cons_idx = thr-threads;
        ring *          r = ioring;
        long            data;
        atomic_u64 *   share;
        atomic_u64 *   priv = &cons_priv[cons_idx];

        printf("cons %p %ld\n", (threadid), thr-threads);

        while (atomic_u64_get(&cons_cnt) < LOOPS || !ring_empty(r)) {
                data = ring_remove(r);
                if (data == ring_invalid(r)) {
                        pthread_yield();
                } else {
                        ASSERT((data >> 56) < NR_PROD_THREADS);
                        share = &prod_share[data>>56];
                        atomic_inc64(&cons_cnt);
                        atomic_inc64(share);
                        atomic_inc64(priv);
			atomic_dec64(&queued);
                        atomic_add64(&consumed_rand, data & 0xffffffff);
                }
        }

        printf("cons %ld consumed %ld\n", thr-threads, atomic_u64_get(priv));
        pthread_exit(NULL);
}

#define LOG_LONG(L)     \
        printf("%s\t:\t%lx\n",#L,L)

int run_insert_remove() 
{
        long i;
        int rc;

        stats_init();
        printf("THREADS cons=%ld prod=%lx LOOPS %ld %ld\n", nr_threads,
               NR_PROD_THREADS, LOOPS, sizeof(b));
        ring_init(ioring, NR_ENTS, DATA_INVALID);

        for (i = 0; i < nr_threads; i++) {
                rc = pthread_create(threads+i, NULL, consumer_func, threads+i);
                if (rc)
                {
                        printf("failed to create consumer thread %ld\n", i);
                        return -1;
                }
        }

        for (; i < nr_threads + NR_PROD_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, producer_func, threads+i);
                if (rc)
                {
                        printf("failed to create producer thread %ld\n", i);
                        return -1;
                }
        }

        for (i = 0; i < MAX_THREADS; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld\n", i);
                        return -1;
                }
        }
        long total_cons = 0;
        long total_prod = 0;
        for (i = 0; i < NR_PROD_THREADS; i++) {
                total_cons += atomic_u64_get(&prod_share[i]);
                total_prod += atomic_u64_get(&prod_priv[i]);
        }

        LOG_LONG(prod_cnt.v);
        LOG_LONG(cons_cnt.v);
        LOG_LONG(total_cons);
        LOG_LONG(total_prod);
        LOG_LONG(produced_rand.v);
        LOG_LONG(consumed_rand.v);
        ASSERT(prod_cnt.v == cons_cnt.v);
        ASSERT(cons_cnt.v == total_cons);
        ASSERT(prod_cnt.v == total_prod);
#ifndef RING_TRACE
        ASSERT(atomic_u64_get(&produced_rand) == atomic_u64_get(&consumed_rand));
#endif
        return 0;
}

static atomic_u64 xchg_var;
static long       xchg_exit;
static long       exit_arg;

static void * xchg_test(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        long            idx = thr-threads;
        long            next = idx;
        long            current = 0;
        long            loops = 0;
        long            putback = 0;

        printf("%s %ld xchg_var %lx\n", __func__, idx, atomic_u64_get(&xchg_var));

        while (current <= xchg_exit)
        {
                loops++;
                current = atomic_xchg64(&xchg_var, -1);
                if (current == next) {
                        printf("%s %ld -> %ld\n", __func__, idx, current);
                        atomic_xchg64(&xchg_var, next+1);
                        next += NR_CONS_THREADS;
                } else if (current == -1) {
                        // nothing wait for other thread to ping-pong back.
                } else {
                        ASSERT(current < next && current > next-NR_CONS_THREADS);
                        atomic_xchg64(&xchg_var, current);
                        putback++;
                }
        }
        printf("%s %ld loops %ld putback %ld\n", __func__, idx, loops, putback);

        pthread_exit(NULL);
}

int test_atomic_xchg() 
{
        long i;
        int rc;

        printf("XCHG THREADS %ld\n", NR_CONS_THREADS);
        atomic_u64_set(&xchg_var, 0);
        xchg_exit = exit_arg ? exit_arg : 10;
        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, xchg_test, threads+i);
                if (rc)
                {
                        printf("failed to create xchg thread %ld\n", i);
                        return -1;
                }
        }

        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld\n", i);
                        return -1;
                }
        }
        return 0;
}

static atomic_u64 xadd_var;
static atomic_u64 xadd_iter;
static long       xadd_exit = 10000000;
static long       xadd_loops[NR_CONS_THREADS];
static void * xadd_test(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        long            idx = thr-threads;
        long            current = atomic_u64_get(&xadd_var);
        long *          loops = &xadd_loops[idx];
        long            val;

        printf("%s %02ld current %016ld\n", __func__, idx, current);

        while (atomic_u64_get(&xadd_iter) < xadd_exit)
        {
                val = rand()*rand();
                if (rand() & 0x1)
                        val *= -1;

                atomic_add64(&xadd_var, val);
                (*loops) += val;
                atomic_inc64(&xadd_iter);
        }
        printf("%s %02ld current %016ld loops %016ld\n", __func__, idx,
                atomic_u64_get(&xadd_iter),
                        *loops);

        pthread_exit(NULL);
}

int test_atomic_xadd() 
{
        long i;
        int rc;
        long total = 0;

        printf("XCHG THREADS %ld\n", NR_CONS_THREADS);
        atomic_u64_set(&xadd_var, 0);
        atomic_u64_set(&xadd_iter, 0);
        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, xadd_test, threads+i);
                if (rc)
                {
                        printf("failed to create xchg thread %ld\n", i);
                        return -1;
                }
        }

        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld\n", i);
                        return -1;
                }
        }
        for (i = 0; i < NR_CONS_THREADS; i++) {
                total += xadd_loops[i];
        }

        printf("xadd_var %ld total %ld\n", atomic_u64_get(&xadd_var), total);
        return 0;
}

static atomic_u64 inc_dec_var;
static atomic_u64 inc_dec_loops;
static long       inc_dec_exit = 100000000;
static long       inc_loops[NR_CONS_THREADS];

static void * atomic_inc_dec(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        long            idx = thr-threads;
        long            current = atomic_u64_get(&inc_dec_var);
        long *          inc = &inc_loops[idx];

        printf("%s %02ld current %016ld\n", __func__, idx, current);

        while (atomic_inc64(&inc_dec_loops) < inc_dec_exit)
        {
                if (rand() & 1) {
                        (*inc)++;
                        atomic_inc64(&inc_dec_var);
                } else {
                        (*inc)--;
                        atomic_dec64(&inc_dec_var);
                }
        }
        printf("%s %02ld inc %016ld\n", __func__, idx, *inc);

        pthread_exit(NULL);
}

int test_atomic_inc_dec() 
{
        long i;
        int rc;
        long total = 0;

        printf("XCHG THREADS %ld\n", NR_CONS_THREADS);
        atomic_u64_set(&inc_dec_var, 0);
        atomic_u64_set(&inc_dec_loops, 0);
        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_create(threads+i, NULL, atomic_inc_dec, threads+i);
                if (rc)
                {
                        printf("failed to create xchg thread %ld\n", i);
                        return -1;
                }
        }

        for (i = 0; i < NR_CONS_THREADS; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld\n", i);
                        return -1;
                }
        }
        for (i = 0; i < NR_CONS_THREADS; i++) {
                total += inc_loops[i];
        }

        printf("inc_dec_var %ld total %ld\n", atomic_u64_get(&inc_dec_var), total);
        return 0;
}

static atomic_u64 fifo_ins_var;
static atomic_u64 fifo_rem_var;
static atomic_u64 fifo_ins_loops;
static atomic_u64 fifo_rem_loops;
static long       fifo_exit = 100;

#define FIFO_SZ 1024
typedef struct fifo_ {
        atomic_u64 data[FIFO_SZ];
        atomic_u64 inuse;

        atomic_u64 ins;
        atomic_u64 ins_priv;

        atomic_u64 rem;
        atomic_u64 rem_priv;
        atomic_u64 rem_lock;

        long       d[NR_THREADS][32];
        long       i[NR_THREADS][32];
} fifo_;

static fifo_ fifo;
static void fifo_lock(fifo_ *f)
{
            while(atomic_xchg64(&f->rem_lock, 1))
                    ;
            //while(__sync_lock_test_and_set(spinlock, 1))
            //                ;
}

static void fifo_unlock(fifo_ *f)
{
            //__sync_lock_release(spinlock);
            atomic_xchg64(&f->rem_lock, 0);
}

static void * atomic_fifo_ins(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        long            idx = thr-threads;
        long            ins = 0;
        long *          fd = fifo.d[idx];
        long *          fi = fifo.i[idx];
        memset(fd, 0, sizeof(long)*32);

        printf("Enter: %s %02ld\n", __func__, idx);

        while (atomic_u64_get(&fifo_ins_loops) < fifo_exit)
        {
                volatile long inuse = atomic_add64(&fifo.inuse, 1);
                COMPILER_BARRIER();
                if (inuse >= FIFO_SZ) {
                        atomic_dec64(&fifo.inuse);
                        continue;
                }
                volatile long ri = atomic_add64(&fifo.ins_priv, 1) - 1;
                long data = ri; //rand();//idx;

                COMPILER_BARRIER();

                long old;
                do {
                        old = atomic_u64_get(&fifo.data[ri & (FIFO_SZ-1)]);
                } while (old != -1);

                COMPILER_BARRIER();
                old = atomic_xchg64(&fifo.data[ri & (FIFO_SZ-1)], data);
                fd[ri & 31] = old;
                fi[ri & 31] = ri;
                ASSERT(old == -1);
                atomic_inc64(&fifo.ins);
                atomic_inc64(&fifo_ins_loops);
                atomic_add64(&fifo_ins_var, data);
                ins++;
                if ((ins & 0x3ffff) == 0) {
                        printf("Ins:  %s %02ld %lx of %lx\n", __func__,
                                idx, atomic_u64_get(&fifo_ins_loops), fifo_exit);
                }
        }
        printf("Exit:  %s %02ld ins %ld of %ld\n", __func__, idx, ins, atomic_u64_get(&fifo_ins_loops));

        pthread_exit(NULL);
}

static void * atomic_fifo_rem(void *threadid)
{
        pthread_t *     thr = (pthread_t *) threadid;
        long            idx = thr-threads;
        long            rem = 0;
        long *          fd = fifo.d[idx];
        long *          fi = fifo.i[idx];
        memset(fd, 0, sizeof(long)*32);

        printf("Enter: %s %02ld\n", __func__, idx);

        while (atomic_u64_get(&fifo_rem_loops) < fifo_exit ||
               atomic_u64_get(&fifo_rem_loops) < atomic_u64_get(&fifo_ins_loops))
        {
                long ri;
                long data;

                fifo_lock(&fifo);
                ri = atomic_inc64(&fifo.rem_priv) - 1;
                if (ri < atomic_u64_get(&fifo.ins)) {
                        do {
                                data = atomic_xchg64(&fifo.data[ri & (FIFO_SZ-1)], -1);
                        } while (data == -1);
                } else {
                        atomic_dec64(&fifo.rem_priv);
                        fifo_unlock(&fifo);
                        continue;
                }
                fifo_unlock(&fifo);

                fd[ri & 31] = data;
                fi[ri & 31] = ri;
                COMPILER_BARRIER();
                atomic_inc64(&fifo.rem);

                COMPILER_BARRIER();

                atomic_dec64(&fifo.inuse);
                atomic_add64(&fifo_rem_var, data);
                atomic_inc64(&fifo_rem_loops);
                rem++;
        }
        printf("Exit:  %s %02ld %ld of %ld\n", __func__, idx, rem, atomic_u64_get(&fifo_rem_loops));

        pthread_exit(NULL);
}

int test_fifo() 
{
        long i;
        int rc;

        printf("%s THREADS %ld\n", __func__, nr_threads);
        atomic_u64_set(&fifo_ins_var, 0);
        atomic_u64_set(&fifo_rem_var, 0);
        atomic_u64_set(&fifo_ins_loops, 0);
        atomic_u64_set(&fifo_rem_loops, 0);

        atomic_u64_set(&fifo.ins, 0);
        atomic_u64_set(&fifo.rem, 0);
        atomic_u64_set(&fifo.ins_priv, 0);
        atomic_u64_set(&fifo.rem_priv, 0);
        atomic_u64_set(&fifo.inuse, 0);
        atomic_u64_set(&fifo.rem_lock, 0);

        fifo_lock(&fifo);
        fifo_unlock(&fifo);

        fifo_exit = exit_arg;
        for (i = 0; i < FIFO_SZ; i++)
        {
                atomic_u64_set(&fifo.data[i], -1);
        }

        for (i = 0; i < nr_threads; i++) {
                if (i & 1) {
                        rc = pthread_create(threads+i, NULL, atomic_fifo_ins,
                                            threads+i);
                } else {
                        rc = pthread_create(threads+i, NULL, atomic_fifo_rem,
                                            threads+i);
                }
                if (rc)
                {
                        printf("failed to create xchg thread %ld\n", i);
                        return -1;
                }
        }

        for (i = 0; i < nr_threads; i++) {
                rc = pthread_join(threads[i], &status[i]);
                if (rc)
                {
                        printf("failed pthread_join %ld\n", i);
                        return -1;
                }
        }
        for (i = 0; i < NR_CONS_THREADS; i++) {
        }

        bool pass =
                atomic_u64_get(&fifo_ins_var) == atomic_u64_get(&fifo_rem_var);
        printf("%s fifo_ins_var %ld fifo_rem_var %ld\n", pass ? "PASS" : "FAIL",
                atomic_u64_get(&fifo_ins_var), atomic_u64_get(&fifo_rem_var));
        pass = atomic_u64_get(&fifo_ins_loops) == atomic_u64_get(&fifo_rem_loops);
        printf("%s fifo_ins_loops %ld fifo_rem_loops %ld\n", pass ? "PASS" : "FAIL",
                atomic_u64_get(&fifo_ins_loops), atomic_u64_get(&fifo_rem_loops));
        return 0;
}


int main(int argc, char **argv)
{
        long loops = 0;
        char c;


        while ((c = getopt (argc, argv, "l:t:")) != -1) {
                switch(c) {
                case 'l':
                        exit_arg = loops = strtoull(optarg, NULL, 0);
                        break;
                case 't':
                        nr_threads = strtoull(optarg, NULL, 0);
                        nr_threads = MIN(nr_threads, NR_CONS_THREADS);
                        break;
                default:
                        return 0;
                }
        }
        srand (time(NULL));

        //test_atomic_xadd();
        //test_atomic_inc_dec();
        //test_atomic_xchg();
        return test_fifo();
        //return 0;

        //while(loops-- > 1) run_insert_remove();
        //return run_insert_remove();
}
