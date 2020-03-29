#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "common.h"
#include "syntax.tab.h"

extern char **IDs;

void resolveProgram(Tree *);

void treeInit(Tree **tp, int st){
  Tree *t = malloc(sizeof(Tree));
  *tp = t;

  for(int i=0;i<MAXCH;++i) t->ch[i] = NULL;
  t->int_val = 0;
  t->float_val = 0;
  t->stype = st;
  t->show = 0;

  /*
  if(st < 250){
    printf("init node of type %s\n", syntaxName[st]);
  }else{
    printf("init token\n");
  }

  if(st != ID) printNode(0, st, t);
  */
}

void printToken(int s, Treep t){
  switch(s){
    case INT: printf("INT: %u\n", t->int_val); break;
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

void treePreDfs(Tree *t){
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
#ifdef PRINTYYTREE
  printNode(x, s, t);
#endif
  for(int i=0;i<MAXCH;++i) if(t->ch[i]) treeDfs(t->ch[i], t->ch[i]->stype, x+1);
}

void treeCompile(Tree *t){
  extern int yylineno;
  treePreDfs(t);
  if(!t->show){
    t->show = 1;
    t->lineno = yylineno;
  }
  treeDfs(t, Program, 0);
  resolveProgram(t);
}
