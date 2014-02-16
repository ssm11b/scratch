#ifndef __RING__H__
#define __RING__H__

typedef struct _ring_meta {
        long prod_idx;
        long cons_idx;
        long inv;
        long size;
} ring_meta;

typedef struct _ring {
        ring_meta       _m;
        long             _data[1];
} ring;

#define RING_INVALID    (_R->_m.inv)
#define RING_SIZE       (_R->_m.size)
#define RING_DATA       (_R->_data)
#define RING_CONS       (_R->_m.cons_idx)
#define RING_PROD       (_R->_m.prod_idx)
#define IDX(i)          (i % RING_SIZE)
#define CONS_IDX        IDX(RING_CONS)
#define PROD_IDX        IDX(RING_PROD)

#define _RING_SZ(_p, _sz)                                               \
        ((long)_sz  - ((long)(_p->_data) - (long)_p)) / sizeof(_p->_data[0])

static inline void ring_init(ring * _R, long sz, long invalid)
{
        int i;
        _R->_m.prod_idx = 0;
        _R->_m.cons_idx = 0;
        _R->_m.inv = invalid;
        _R->_m.size = _RING_SZ(_R, sz);
        for (i = 0; i < RING_SIZE; i++)
        {
                RING_DATA[i] = RING_INVALID;
        }
}

static inline int ring_empty(ring * _R)
{
        return PROD_IDX == CONS_IDX;
}

static inline int ring_full(ring * _R)
{
        return (PROD_IDX - CONS_IDX) == RING_SIZE;
}

static void ring_dump(ring * _R)
{
        printf("ring %p[%ld]: E=%d F=%d\n", _R, RING_SIZE, ring_empty(_R), ring_full(_R));
}


#define ring_idx_valid(r, idx) 1
#define LOCK_PREFIX "lock; "
static inline long xadd64(long *ptr_, long inc)
{
        long ret_ = inc;
        asm volatile (LOCK_PREFIX "xaddq %q0, %1\n"     \
                      : "+r" (ret_), "+m" (*(ptr_))      \
                      : : "memory", "cc");              \
        return ret_;
}

static inline long atomic_add64(long *v, long i)
{
	return i + xadd64(v, i);
}

static inline long atomic_xchg64(long * ptr_, long new_)
{
        long ret_ = new_;
        asm volatile (LOCK_PREFIX "xchgq %q0, %1\n"     \
                      : "+r" (ret_), "+m" (*(ptr_))      \
                      : : "memory", "cc");              \
        return ret_;
}

#define atomic_inc64(v) atomic_add64(v, 1)
#define atomic_dec64(v) atomic_add64(v, -1)

static int ring_insert(ring * _R, long data)
{
        long idx;
try_insert:
        if (ring_full(_R)) {
                return RING_INVALID;
        }

        idx = atomic_inc64(&_R->_m.prod_idx) - 1;
        // atomic_inc_return64 : atomic increment and return old value
        if (!ring_idx_valid(_R, idx))
        {
                atomic_dec64(&_R->_m.prod_idx);
                goto try_insert;
        }

        _R->_data[idx % _R->_m.size] = data;
        return 0;
}

/*
 * ring_remove
 *
 * returns index of entry that was removed, RING_INVALID if none.
 *
 */
static long ring_remove(ring * _R)
{
        long data = RING_INVALID;
again:
        if (ring_empty(_R)) {
                return RING_INVALID;
        }
        data = atomic_xchg64(&_R->_data[CONS_IDX]/*_R->_m.cons_idx]*/, RING_INVALID);
        if (data == RING_INVALID) {
                goto again;
        }
        atomic_inc64(&_R->_m.cons_idx);
        return data ;
}

#endif
