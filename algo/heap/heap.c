#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(x, y) ((x) < (y) ? (x) : (y))

typedef struct {
  uint64_t* data;
  size_t data_words;;
  size_t size;
} heap_t;


static void
heap_init(heap_t* heap, uint64_t* data, size_t size)
{
  heap->data = data;
  heap->data_words = size;
  heap->size = size;
}

static heap_t*
heap_create(size_t size)
{
  size_t malloc_size = sizeof(heap_t) + size * sizeof(uint64_t);
  heap_t* heap = malloc(malloc_size);
  if (heap == NULL) {
    return NULL;
  }

  heap->data = malloc(size * sizeof(uint64_t));
  if (!heap->data) {
    free(heap);
    return NULL;
  }

  heap->data_words = size;
  heap->size = 0;
  return heap;
}

static size_t
heap_size(heap_t* h)
{
  assert(h->size <= h->data_words);
  return h->size;
}

static size_t
heap_parent(size_t child_index)
{
  return child_index ? (child_index - 1) / 2 : 0;
}

static size_t
heap_left(size_t parent)
{
  return parent * 2 + 1;
}

static size_t
heap_right(size_t parent)
{
  return heap_left(parent) + 1;
}

static uint64_t
heap_get(heap_t* h, size_t index)
{
  if (index < heap_size(h)) {
    return h->data[index];
  }
  return 0;
}

static void
heap_set(heap_t* h, uint64_t data, size_t index)
{
  if (index < heap_size(h)) {
    h->data[index] = data;;
  }
}

static void
heap_exchange(heap_t* h, size_t i, size_t j)
{
  assert(i < heap_size(h));
  assert(j < heap_size(h));

  uint64_t data_i = heap_get(h, i);
  uint64_t data_j = heap_get(h, j);
  heap_set(h, data_i, j);
  heap_set(h, data_j, i);
}

static void
heap_swim(heap_t* h, size_t index)
{
  size_t parent = heap_parent(index);
  while (index != 0 && heap_get(h, index) > heap_get(h, parent)) {
    heap_exchange(h, index, parent);
    index = parent;
    parent = heap_parent(index);
  }
}

static void
heap_insert(heap_t* h, uint64_t data)
{
  size_t at = h->size++;
  h->data[at] = data;
  heap_swim(h, at);
}

static void
heap_sink(heap_t* h, size_t index)
{
  while (index < heap_size(h)) {
    if (heap_get(h, heap_right(index)) > heap_get(h, heap_left(index)) &&
        heap_get(h, index) < heap_get(h, heap_right(index))) {
      /* If right is greater than left, we know both right and left are valid,
       * i.e. (!= 0), check if right > index, if so swap with the right.
       */
      heap_exchange(h, index, heap_right(index));
      index = heap_right(index);
    } else if (heap_get(h, index) < heap_get(h, heap_left(index))) {
      /* If left is valid (!= 0) and greater than index swap. */
      heap_exchange(h, index, heap_left(index));
      index = heap_left(index);
    } else {
      /* Neither right or left were swapped, break out. */
      break;
    }
  }
}

static uint64_t
heap_remove(heap_t* h)
{
  heap_exchange(h, 0, heap_size(h) - 1);
  uint64_t data = heap_get(h, heap_size(h) - 1);
  h->size--;
  heap_sink(h, 0);
  return data;
}

void
heap_dump(heap_t* h)
{
  size_t level = 0;
  while ((1 << level) <= heap_size(h)) {
    size_t num = 1 << level;
    size_t start = num - 1;
    char space[256];
    memset(space, ' ', level*2);
    space[level*2] = '\0';

    for (size_t i = start; i < heap_size(h) && i < start + num; i++) {
      printf("[%zu, %03zu] %s %lld\n", level, i, space, h->data[i]);
    }
    level += 1;
  }
}

static bool
heap_verify(heap_t* h)
{
  for (size_t i = 0; i < heap_size(h); i++) {
    uint64_t data = heap_get(h, i);
    if (data < heap_get(h, heap_left(i)) || data < heap_get(h, heap_right(i))) {
#if 1
      printf("[%zu] %lld, l -> [%zu] %lld r -> [%zu] %lld\n",
          i, data,
          heap_left(i), heap_get(h, heap_left(i)),
          heap_right(i), heap_get(h, heap_right(i)));
#endif
      return false;
    }
  }
  return true;
}

static void
heap_test_verify()
{
  heap_t heap;

  {
    uint64_t data[] = { 3, 2, 1 };
    heap_init(&heap, data, 3);
    //heap_dump(&heap);
    assert(heap_verify(&heap));
  }
  {
    uint64_t data[] = { 3, 2, 1, 4 };
    heap_init(&heap, data, 4);
    //heap_dump(&heap);
    assert(!heap_verify(&heap));
  }
}

static const size_t c_heap_size = 16 * 1024;

int main()
{
  heap_test_verify();
  heap_t* h =  heap_create(c_heap_size);
  srandom((long)h);

  assert(h);
  assert(h->data_words == c_heap_size);
  assert(heap_size(h) == 0);

  for (size_t i = 0; i < c_heap_size; i++) {
    heap_insert(h, random());
    // heap_dump(h);
    assert(heap_verify(h));
  }

  // heap_dump(h);

  uint64_t max = 0;
  for (size_t i = 0; i < c_heap_size; i++) {
    uint64_t data = heap_remove(h);
    if (!heap_verify(h)) {
      heap_dump(h);
    }
    // printf("[%zu] %lld <= %lld\n", i, data, max);
    assert(i == 0 || data <= max);
    max = data;
  }

  assert(heap_size(h) == 0);

}
