#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "common.h"
#include "syntax.tab.h"

extern char *IDs[];

void treeInit(Tree **tp, int st){
  Tree *t = malloc(sizeof(Tree));
  *tp = t;

  for(int i=0;i<MAXCH;++i) t->ch[i] = NULL;
  t->int_val = 0;
  t->float_val = 0;
  t->stype = st;
  t->show = 0;

  t->lineno = 0;
  t->errmsg = NULL;

  /*
  if(st < 250){
    printf("init node of type %s\n", syntaxName[st]);
  }else{
    printf("init token\n");
  }
  */
}

void printToken(int s, Treep t){
  switch(s){
    case INT: printf("INT: %d\n", t->int_val); break;
    case FLOAT: printf("FLOAT: %f\n", t->float_val); break;
    case ID: printf("ID: %s\n", IDs[t->int_val]); break;
    case SEMI: printf("SEMI\n"); break;
    case COMMA: printf("COMMA\n"); break;
    case TYPE: printf("TYPE: %s\n", t->int_val ? "float" : "int"); break;
    case LC: printf("LC\n"); break;
    case RC: printf("RC\n"); break;
    case STRUCT: printf("STRUCT\n"); break;
    case RETURN: printf("RETURN\n"); break;
    case IF: printf("IF\n"); break;
    case ELSE: printf("ELSE\n"); break;
    case WHILE: printf("WHILE\n"); break;
    case ASSIGNOP: printf("ASSIGNOP\n"); break;
    case OR: printf("OR\n"); break;
    case AND: printf("AND\n"); break;
    case RELOP: printf("RELOP\n"); break;
    case PLUS: printf("PLUS\n"); break;
    case MINUS: printf("MINUS\n"); break;
    case STAR: printf("STAR\n"); break;
    case DIV: printf("DIV\n"); break;
    case NOT: printf("NOT\n"); break;
    case LP: printf("LP\n"); break;
    case RP: printf("RP\n"); break;
    case LB: printf("LB\n"); break;
    case RB: printf("RB\n"); break;
    case DOT: printf("DOT\n"); break;
    default: assert(0);
  }
}

void printNode(int x, int s, Tree *t){
  for(int i=0;i<x;++i) printf("  ");
  if(s > 250){
    printToken(s, t);
  }else{
    printf("%s (%d)\n", syntaxName[s], t->lineno);
  }
}

Tree **msgs = NULL;
int msgs_size = 0, msgs_cnt = 0;
void addErrMsg(Tree *t){
  assert(msgs_size >= msgs_cnt);
  if(msgs_size == msgs_cnt){
    Tree **nmsgs = realloc(msgs, sizeof(Tree *) * (msgs_size * 2 + 1));
    for(int i=0;i<msgs_size;++i) nmsgs[i] = msgs[i];
    msgs = nmsgs;

    msgs_size = msgs_size * 2 + 1;
  }

  assert(msgs_size > msgs_cnt);
  msgs[msgs_cnt++] = t;
}
int cmpErrMsg(const void *app, const void *bpp){
  const Treep *ap = app, *bp = bpp;
  Treep a = *ap, b = *bp;
  if(a->errlineno < b->errlineno) return -1;
  else if(a->errlineno == b->errlineno) return 0;
  else return 1;
}
void sortErrMsgs(){
  qsort(msgs, msgs_cnt, sizeof(Tree *), cmpErrMsg);
}
void printErrMsgs(){
  for(int i=0;i<msgs_cnt;++i){
    Tree *t = msgs[i];
    printf("Error type %c at Line %d: %s\n", t->errtype ? 'B':'A', t->errlineno, t->errmsg);
  }
}

void treePreDfs(Tree *t){
  if(t->errmsg){
    addErrMsg(t);
  }
  if(t->stype >= INT){
    t->show = 1;
    return;
  }

  int foundch = 0;
  for(int i=0;i<MAXCH;++i) if(t->ch[i]){
    treePreDfs(t->ch[i]);
    if(t->ch[i]->show){
      t->show = 1;

      if(foundch){
      }else{
        foundch = 1;
        t->lineno = t->ch[i]->lineno;
      }

    }
  }
}

void treeDfs(Tree *t, int s, int x){
  //printf("dfsing %d, %d\n", (int)t, s);
  if(!t->show) return;
  printNode(x, s, t);
  for(int i=0;i<MAXCH;++i) if(t->ch[i]) treeDfs(t->ch[i], t->ch[i]->stype, x+1);
}

void treePrint(Tree *t){
  treePreDfs(t);

  if(msgs){
    sortErrMsgs();
    printErrMsgs();
  }else{
    treeDfs(t, Program, 0);
  }
}
