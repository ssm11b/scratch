#include <stdio.h>
#include <stdlib.h>

typedef struct stack_node stack_node_t;

typedef struct stack_node {
  stack_node_t* next;
} stack_node_t;

typedef struct stack {
  stack_node_t* top;
  size_t size;
} stack_impl_t;

static void
stack_push(stack_impl_t* stack, stack_node_t* node)
{
  node->next = stack->top;
  stack->top = node;
  stack->size++;
}

static stack_node_t*
stack_pop(stack_impl_t* stack)
{
  stack_node_t* node = NULL;
  if (stack->top) {
    node = stack->top;
    stack->top = stack->top->next;
    stack->size--;
  }
  return node;
}

static size_t
stack_size(stack_impl_t* stack)
{
  return stack->size;
}

static void
stack_init(stack_impl_t* stack)
{
  stack->top = NULL;
  stack->size = 0;
}


typedef struct {
  stack_node_t node;
  int data;
} node_int_t;

int main()
{
  printf("hello stack\n");

  stack_impl_t stack;
  stack_init(&stack);

  for (int i = 0; i < 10; i++) {
    node_int_t* node = malloc(sizeof(node_int_t));
    node->data = i*2;
    stack_push(&stack, &node->node);
  }

  while (stack_size(&stack) != 0) {
    node_int_t* node = (node_int_t*)stack_pop(&stack);
    printf("%d ", node->data);
    free(node);
  }
  printf("\n");
}
