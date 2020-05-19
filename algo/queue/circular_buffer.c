#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define min(x, y) ((x) < (y) ? (x) : (y))

typedef struct {
  char* data;
  size_t size;
  size_t count;
  size_t head;
  size_t tail;
} queue_t;


static bool
queue_init(queue_t* queue, size_t size)
{
  queue->data = malloc(size);
  if (queue->data == NULL) {
    return false;
  }
  queue->size = size;
  queue->count = 0;
  queue->head = 0;
  queue->tail = 0;

  return true;
}

static size_t
queue_enqueue(queue_t* queue, const char* data, size_t data_len)
{
  size_t bytes = min(data_len, queue->size - queue->count);

  if (queue->tail + bytes < queue->size) {
    memcpy(queue->data + queue->tail, data, bytes);
    queue->tail += bytes;
  } else {
    size_t delta = queue->size - queue->tail;
    memcpy(queue->data + queue->tail, data, delta);
    memcpy(queue->data, data + delta, bytes - delta);
    queue->tail = bytes - delta;
  }
  queue->count += bytes;

  return bytes;
}

static size_t
queue_dequeue(queue_t* queue, char* data, size_t data_len)
{
  size_t bytes = min(data_len, queue->count);

  if (queue->count == 0) {
    return 0;
  }

  if (queue->head + bytes < queue->size) {
    memcpy(data, queue->data + queue->head, bytes);
    queue->head += bytes;
  } else {
    size_t delta = queue->size - queue->head;
    memcpy(data, queue->data + queue->head, delta);
    memcpy(data + delta, queue->data, bytes - delta);
    queue->head = bytes - delta;
  }
  queue->count -= bytes;

  return bytes;
}

const char* str1 = "hello queue one";
const char* str2 = "hello queue two";
const char* str3 = "hello queue three";
const char* str4 = "01234567890123456789012345";
int main()
{
  char data[1024];
  queue_t q;


  queue_init(&q, 24);

  {
    size_t size = strlen(str1);
    assert(queue_enqueue(&q, str1, size) == size);
    //printf("dequeue: %zu\n", queue_dequeue(&q, data, size));
    assert(queue_dequeue(&q, data, size) == size);
    assert(strncmp(data, str1, size) == 0);
    data[size] = 0;
    printf("%s\n", data);
  }
  {
    size_t size = strlen(str2);
    assert(queue_enqueue(&q, str2, size) == size);
    assert(queue_dequeue(&q, data, size) == size);
    assert(strncmp(data, str2, size) == 0);
    data[size] = 0;
    printf("%s\n", data);
  }

  {
    size_t size = strlen(str3);
    assert(queue_enqueue(&q, str3, size) == size);
    assert(queue_dequeue(&q, data, size) == size);
    assert(strncmp(data, str3, size) == 0);
    data[size] = 0;
    printf("%s\n", data);
  }
  {
    size_t size = strlen(str4);
    assert(queue_enqueue(&q, str4, size) == 24);
    assert(queue_dequeue(&q, data, size) == 24);
    assert(strncmp(data, str4, 24) == 0);
    data[24] = 0;
    printf("%s\n", data);
  }
}
