// #pragme once - in this scenario we want to allow multiple loading - it might
// be that first time we loaded with no defines, but next time we might have
// needed defines

// {
#ifdef CIRCULAR_DOUBLY_LINKED_LIST // HEADER
#pragma once

#include <stdbool.h>

typedef struct CDLLNode {
  bool (*cmp)(void *, void *);
  void (*print)(struct CDLLNode *node);
  void (*free_data)(void *); // frees memory
  struct CDLLNode *prev;
  struct CDLLNode *next;
  void *data;
} CDLLNode;

// cdll_list prints the full linked list
void cdll_list(CDLLNode *cursor);

// cdll_list_len finds the linked_list length
size_t cdll_list_len(CDLLNode *cursor);

// cdll_free_node frees the current node and returns the next linked list
// element frees memory
CDLLNode *cdll_node_free(CDLLNode *cursor);

// cdll_free_nodes frees the entire linked list
// frees memory
void cdll_nodes_free(CDLLNode *cursor);

// cdll_create prepares a new node with all the helper functions
// returns NULL if at least one argument is not properly provided
// allocs memory
CDLLNode *cdll_create(bool (*cmp)(void *, void *),
                      void (*print)(CDLLNode *node),
                      void (*free_data)(void *data), CDLLNode *prev,
                      CDLLNode *next, void *data);

// cdll_append creates a new node at the end of the linked list
// returns NULL if cursor is NULL
// allocs memory
CDLLNode *cdll_append(CDLLNode *cursor, void *data);

/*
// cdll_find searches linked list for provided data and returns the first found
// instance
// returns NULL if cursor is NULL
CDLLNode *cdll_find(CDLLNode *cursor, void *data);

// cdll_update replaces old_data with new_data on the first found instance
// frees data
// returns NULL if cursor is NULL
CDLLNode *cdll_update(CDLLNode *cursor, void *old_data, void *new_data);
*/

// cdll_delete removes the node from the linked list
// linked list is kept intact
// returns NULL if cursor is NULL
// frees memory
CDLLNode *cdll_delete(CDLLNode *cursor, void *data);

#endif // CIRCULAR_DOUBLY_LINKED_LIST // HEADER
// }

// {
#ifdef CIRCULAR_DOUBLY_LINKED_LIST // IMPLEMENTATION
#pragma once

#include <stdlib.h>

void cdll_list(CDLLNode *cursor) {
  if (!cursor) {
    printf("-> %p\n", (void *)cursor);
    return;
  }
  CDLLNode *cur = cursor;
  printf("-> ");
  cur->print(cur);
  for (cur = cur->next; cur && cur != cursor; cur = cur->next) {
    printf("-> ");
    cur->print(cur);
  }
  return;
}

size_t cdll_list_len(CDLLNode *cursor) {
  if (!cursor) {
    return 0;
  }
  CDLLNode *cur = cursor;
  cur = cur->next;
  for (size_t i = 1; cur; ++i, cur = cur->next) {
    if (cur == cursor) {
      return i;
    }
  }
  return 0;
}

CDLLNode *cdll_node_free(CDLLNode *cursor) {
  if (!cursor) {
    return NULL;
  }
  // comment in for logging: printf("freeing (%p)\n", cursor);
  CDLLNode *me = cursor;
  if (me->prev) {
    me->prev->next = NULL;
  }
  me->prev = NULL;
  cursor = cursor->next;
  me->free_data(me->data);
  free(me);
  if (cursor) {
    cursor->prev = NULL;
  }
  return cursor;
}

void cdll_nodes_free(CDLLNode *cursor) {
  for (; cursor != NULL;) {
    cursor = cdll_node_free(cursor);
  }
  return;
}

CDLLNode *cdll_create(bool (*cmp)(void *, void *),
                      void (*print)(CDLLNode *node),
                      void (*free_data)(void *data), CDLLNode *prev,
                      CDLLNode *next, void *data) {
  if (!cmp || !print || !free_data || !data) {
    return NULL;
  }
  CDLLNode *new = calloc(1, sizeof(CDLLNode));
  if (!prev) {
    prev = new;
  }
  if (!next) {
    next = new;
  }
  new->cmp = cmp;
  new->print = print;
  new->free_data = free_data;
  new->data = data;
  new->next = next;
  new->prev = prev;
  return new;
}

CDLLNode *cdll_append(CDLLNode *cursor, void *data) {
  if (!cursor) {
    return NULL;
  }

  CDLLNode *cur = cursor;
  for (; cur->next != cursor; cur = cur->next) {
    ;
  }
  CDLLNode *new = cdll_create(cursor->cmp, cursor->print, cursor->free_data,
                              cur, cur->next, data);
  cur->next = new;
  cursor->prev = new;
  return cursor;
}

#endif // CIRCULAR_DOUBLY_LINKED_LIST // IMPLEMENTATION
// }
