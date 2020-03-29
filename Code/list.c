#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

ListNode *newListNode(void *v){
  ListNode *ret = malloc(sizeof(ListNode));
  ret->last = ret->next = NULL;
  ret->val = v;
}

void listAppend(List *l, void *v){
  if(l->head == NULL){
    assert(l->rear == NULL);
    l->head = l->rear = newListNode(v);
  }else{
    ListNode *n = newListNode(v);
    l->rear->next = n;
    n->last = l->rear;
    l->rear = n;
  }
}
