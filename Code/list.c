#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

ListNode *newListNode(void *v){
  ListNode *ret = malloc(sizeof(ListNode));
  ret->prev = ret->next = NULL;
  ret->val = v;
}

List *newList(){
  List *list = malloc(sizeof(List));
  list->head = list->rear = NULL;
  return list;
}

void listAppend(List *l, void *v){
  if(l->head == NULL){
    assert(l->rear == NULL);
    l->head = l->rear = newListNode(v);
  }else{
    ListNode *n = newListNode(v);
    l->rear->next = n;
    n->prev = l->rear;
    l->rear = n;
  }
}

void listPopRear(List *l){ //TODO: maybe we can free space here
  assert(l->head);
  assert(l->rear);
  if(l->head == l->rear){
    l->head = l->rear = NULL;
  }else{
    l->rear = l->rear->prev;
  }
}

List *listMerge(List *a, List *b){
  if(a->head == NULL) return b;
  else if(b->head == NULL) return a;
  else{
    a->rear->next = b->head;
    b->head->prev = a->rear;
    a->rear = b->rear;
    return a;
  }
}
