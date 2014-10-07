/*
 *
 * This program is free software; you may redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * This file defines a lockless circular FIFO ring, which can accessed
 * atomically from multiple produce and consumer threads.
 */
#ifndef __RING_H
#define __RING_H

#if !defined(__KERNEL__)
#define LOCK_PREFIX "lock; "
#define ASM __asm

#ifndef ASSERT
#define ASSERT
#endif

#else   // __KERNEL__

#define ASSERT(COND) WARN_ON(!(COND))
#define ASM asm

#endif  // __KERNEL__

#define COMPILER_BARRIER() ASM volatile("" ::: "memory")
#define __CACHE_ALIGN__ __attribute__((aligned(64)))

typedef struct {
        long v;
} atomic_u64;


static long atomic_u64_get(atomic_u64 * i)
{
        return (*(volatile long *)&(i)->v);
}

static void atomic_u64_set(atomic_u64 * i, long val)
{
        i->v = val;
}

/*
 * ring_priv -
 *
 * Ring private data, meta data describing state of the shared ring.
 *
 * The shared ring implements MPMC (multiple producers multiple consumer)
 * FIFO. MPMC is acheived through atomic operations on shared ring header and
 * data array.
 *
 * Both the producer and consumer have a private and public indices into the
 * ring. The public index is viewable across both producers and consumers,
 * while the private index is not. The private index is used to operate on the
 * ring locally and once the operation has completed, it is propagated to the
 * public index.
 *
 * The size of the ring must be a power of 2 for proper masking of the ring
 * indices.
 */
typedef struct _ring_priv {
        atomic_u64 prod_idx     __CACHE_ALIGN__; // producer index
        atomic_u64 prod_prv     __CACHE_ALIGN__; // producer private index
        atomic_u64 cons_idx     __CACHE_ALIGN__; // consumer index
        atomic_u64 cons_prv     __CACHE_ALIGN__; // consumer private index
        atomic_u64 cons_lock    __CACHE_ALIGN__; // FIXME: monitor lock for consumer
        atomic_u64 inuse        __CACHE_ALIGN__; // number of inuse ring entries
        long size; // ring depth, must be a power of 2
        long mask; // mask used to find ring entry
        long inv;  // invalid index
        long pad __CACHE_ALIGN__;
} ring_priv;

/*
 * ring -
 *
 * Shared ring buffer that provides MPMC semantics.
 *
 */
typedef struct ring {
        ring_priv    hdr;
        atomic_u64      _data[1];
} ring;

/*
 * Internal macros.
 *
 */
#define RING_INVALID    (_R->hdr.inv)
#define RING_SIZE       (_R->hdr.size)
#define RING_MASK       (_R->hdr.mask)
#define RING_INUSE      atomic_u64_get(&_R->hdr.inuse)

#define RING_SIZE_PAGE_ALIGN(X) (((X)+0xfff) & ~0xfff)
#define RING_ALLOC_SIZE(NR_ENTS)        \
        RING_SIZE_PAGE_ALIGN(sizeof(ring_priv) + sizeof(long)*NR_ENTS)

/*
 * Initialize the ring at address _R, SZ indicated the number of bytes
 * for the entire ring data structures, the actual ring depth is determined
 * based on  SZ- sizeof(ring_priv), INVALID indicates an invalid data
 * entry in the ring.
 */
static inline void ring_init(ring * _R, long sz, long invalid)
{
        int i;
        atomic_u64_set(&_R->hdr.prod_idx, 0);
        atomic_u64_set(&_R->hdr.cons_idx, 0);
        atomic_u64_set(&_R->hdr.prod_prv, 0);
        atomic_u64_set(&_R->hdr.cons_prv, 0);
        atomic_u64_set(&_R->hdr.inuse, 0);
        atomic_u64_set(&_R->hdr.cons_lock, 0);
        _R->hdr.inv             = invalid;
        _R->hdr.size            = sz;
        _R->hdr.mask            = sz-1;

        ASSERT((RING_SIZE & RING_MASK) == 0);
        for (i = 0; i < RING_SIZE; i++)
        {
                atomic_u64_set(&_R->_data[i], RING_INVALID);
        }
}

static inline int ring_empty(ring * _R)
{
        return RING_INUSE == 0;
}

static inline int ring_full(ring * _R)
{
        return RING_INUSE == RING_SIZE;
}

/*
 * These atomic operation have been cherry picked from the kernel/x86_64
 * atomics implementation.  They are repilicated here so the atomic
 * interface can be used by both kernel and user code.
 */

/*
 * Atomically exchange and ADD 64-bit value, returns the value prior to
 * the additon.
 */
static inline long xadd64(long * ptr_, long inc)
{
        long ret_ = inc;
        ASM volatile (LOCK_PREFIX "xaddq %q0, %1\n"     \
                      : "+r" (ret_), "+m" (*(ptr_))     \
                      : : "memory", "cc");              \
        return ret_;
}

/*
 * Same as atomic64_add_return in the kernel.
 */
static inline long atomic_add64(atomic_u64 * idx, long i)
{
	return i + xadd64(&idx->v, i);
}

/*
 * Same as atomic64_xchng in the kernel.
 */
static inline long atomic_xchg64(atomic_u64 * idx, long new_)
{
        long ret_ = new_;
        ASM volatile ("xchgq %q0, %1\n"                 \
                      : "+r" (ret_), "+m" ((idx->v))    \
                      : : "memory", "cc");              \
        return ret_;
}

#define atomic_inc64(v) atomic_add64(v, 1)
#define atomic_dec64(v) atomic_add64(v, -1)

#ifdef RING_TRACE
#define TRACE_BUFFER_SZ 16 
long trace_cidx[TRACE_BUFFER_SZ];
long trace_cdata[TRACE_BUFFER_SZ];
long trace_pidx[TRACE_BUFFER_SZ];
long trace_pdata[TRACE_BUFFER_SZ];
#endif

#ifdef RING_PROD
/*
 * ring_insert -
 *
 * Atomically insert the DATA into the ring, if the ring is full return
 * RING_INVALID, else 0 for success.
 *
 */
static int ring_insert(ring * _R, long data)
{
        long idx, used;
        long old;

        // _R->hdr.inuse is used as a barrier to only allow atmost the N number
        // of producers to proceed, where N the number of elements empty in the
        // queue.
        used = atomic_add64(&_R->hdr.inuse, 1);
        if (used > RING_SIZE) {
                goto exit_full;
        }
        COMPILER_BARRIER();

        // allocate a prod_idx, each producer allocates a slot atomically from
        // the prod_prv and publishes the allocations as completed as a last
        // step in insert.
        idx = atomic_add64(&_R->hdr.prod_prv, 1) - 1;
        COMPILER_BARRIER();

        // the consumer might in the process of removing the current element,
        // there is slight race here when multiple conusmer are trying to
        // remove an element, if producer A is removing from the current slot
        // and producer B leaps A and exit first, i.e. bumping CONS_IDX.
        do {
                old = atomic_u64_get(&_R->_data[idx & RING_MASK]);
        } while (old != RING_INVALID);
        COMPILER_BARRIER();

#ifdef RING_TRACE
	trace_pdata[idx % TRACE_BUFFER_SZ] = idx;
	trace_pidx[idx % TRACE_BUFFER_SZ] = idx;
#endif // RING_TRACE

        old = atomic_xchg64(&_R->_data[idx & RING_MASK], data);
        ASSERT(old == RING_INVALID);
        atomic_inc64(&_R->hdr.prod_idx);
        return 0;

exit_full:
        atomic_dec64(&_R->hdr.inuse);
        return RING_INVALID;
}
#endif

#ifdef RING_CONS

static void monitor_enter(atomic_u64 * var)
{
        while(atomic_xchg64(var, 1))
                ;
}

static void monitor_exit(atomic_u64 * var)
{
        atomic_xchg64(var, 0);
}

/*
 * ring_remove -
 *
 * Removes next data ring buffer returns the data. If the ring is empty,
 * returns RING_INVALID.
 *
 */
static long ring_remove(ring * _R)
{
	long ci, data;

        // TODO: FIXME
        // There is a race here when multiple consumers examine the producer
        // index and remove, not clear the exact issue still needs to be root
        // caused.  Adding the this lock to serialize comsumers through this
        // section.     -sandeep
        monitor_enter(&_R->hdr.cons_lock);
        ci = atomic_inc64(&_R->hdr.cons_prv) - 1;
        if (ci < atomic_u64_get(&_R->hdr.prod_idx)) {
                do {
                        data = atomic_xchg64(&_R->_data[ci & RING_MASK],
                                             RING_INVALID);
                } while (data == -1);
        } else {
                goto exit_empty;
        }
        monitor_exit(&_R->hdr.cons_lock);

#ifdef RING_TRACE
	trace_cidx[ci % TRACE_BUFFER_SZ] = ci;
	trace_cdata[ci % TRACE_BUFFER_SZ] = data;
#endif // RING_TRACE

        atomic_inc64(&_R->hdr.cons_idx);
        COMPILER_BARRIER();
        atomic_dec64(&_R->hdr.inuse);
        return data;

exit_empty:
        atomic_dec64(&_R->hdr.cons_prv);
        monitor_exit(&_R->hdr.cons_lock);
        return RING_INVALID;
}

static long ring_invalid(ring * _R)
{
        return RING_INVALID;
}
#endif

#endif
