#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

Vector *vector_new();
void vec_pb(Vector *, IRCode);
SymTabEntry *trieQuery(Trie *, const char *);

extern Trie *symTabFunctions;
extern FILE *fir;
extern char **IDs;

static Vector *ir_code;
static int label_cnt = 0;

static void codes1(int op, int dst, const char *src1_var, int src2){
  IRCode irc;
  irc.op = op;
  irc.dst = dst;
  irc.src1_var = src1_var;
  irc.src2 = src2;
  vec_pb(ir_code, irc);
}

static void coded(int op, const char *dst_var, int src1, int src2){
  IRCode irc;
  irc.op = op;
  irc.dst_var = dst_var;
  irc.src1 = src1;
  irc.src2 = src2;
  vec_pb(ir_code, irc);
}

static void code(int op, int dst, int src1, int src2){
  IRCode irc;
  irc.op = op;
  irc.dst = dst;
  irc.src1 = src1;
  irc.src2 = src2;
  vec_pb(ir_code, irc);
}

static void printIR(IRCode ir){
  switch(ir.op){
    case OP_FUNCTION:
      fprintf(fir, "FUNCTION %s :\n", ir.src1_var);
      break;
    case OP_PARAM:
      fprintf(fir, "PARAM %s\n", ir.src1_var);
      break;
    case OP_FUNCALL:
      fprintf(fir, "t%d := CALL %s\n", ir.dst, ir.src1_var);
      break;
    case OP_ARG:
      fprintf(fir, "ARG t%d\n", ir.src1);
      break;
    case OP_READ:
      fprintf(fir, "READ t%d\n", ir.dst);
      break;
    case OP_WRITE:
      fprintf(fir, "WRITE t%d\n", ir.src1);
      break;
    case OP_LOAD:
      fprintf(fir, "t%d := %s\n", ir.dst, ir.src1_var);
      break;
    case OP_STORE:
      fprintf(fir, "%s := t%d\n", ir.dst_var, ir.src1);
      break;
    case OP_LOAD_IMM:
      fprintf(fir, "t%d := #%d\n", ir.dst, ir.src1);
      break;
    case OP_ADD:
      fprintf(fir, "t%d := t%d + t%d\n", ir.dst, ir.src1, ir.src2);
      break;
    case OP_RETURN:
      fprintf(fir, "RETURN t%d\n", ir.src1);
      break;
    default:
      fprintf(stderr, "not implemented!\n");
      assert(0);
  }
}

static void printIRCode(){
  for(int i=0;i<ir_code->len;++i){
    printIR(ir_code->data[i]);
  }
}

static void irCompst(Tree *);
static void irStmtList(Tree *);
static void irStmt(Tree *);
static void irDefList(Tree *); //initialize variables
static void irDef(Tree *);
static void irDecList(Tree *);
static void irExp(Tree *); //syn: label
static void irArgs(Tree *);

static void irCompSt(Tree *t){
  irDefList(t->ch[1]);
  irStmtList(t->ch[2]);
}

static void irStmtList(Tree *t){
  if(t->ch[0]){
    irStmt(t->ch[0]);
    irStmtList(t->ch[1]);
  }
}

static void irStmt(Tree *t){
  switch(t->int_val){
    case Stmt_Exp:
      irExp(t->ch[0]);
      break;
    case Stmt_Return:
      irExp(t->ch[1]);
      code(OP_RETURN, 0, t->ch[1]->label, 0);
      break;
    default:
      assert(0);
  }
}

static void irDefList(Tree *t){
  if(t->ch[0]){
    irDef(t->ch[0]);
    irDefList(t->ch[1]);
  }
}

static void irDef(Tree *t){
  irDecList(t->ch[1]);
}

static void irDecList(Tree *t){
  //TODO;
}

static void irExp_FunCall(Tree *t){
  const char *funName = IDs[t->ch[0]->int_val];

  if(strcmp(funName, "read") == 0){
    code(OP_READ, t->label, 0, 0);
    return;
  }else if(strcmp(funName, "write") == 0){
    assert(t->ch[2]);
    irExp(t->ch[2]->ch[0]);
    code(OP_WRITE, 0, t->ch[2]->ch[0]->label, 0);
    return;
  }

  if(t->ch[2]){
    irArgs(t->ch[2]);
  }

  codes1(OP_FUNCALL, t->label, funName, 0);
}

static void irExp(Tree *t){
  t->label = ++label_cnt;
  switch(t->int_val){
    case Exp_ASSIGN:
      irExp(t->ch[2]);
      switch(t->ch[0]->int_val){
        case Exp_Id:
          coded(OP_STORE, IDs[t->ch[0]->ch[0]->int_val], t->ch[2]->label, 0);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case Exp_AND:
      assert(0);
      break;
    case Exp_OR:
      assert(0);
      break;
    case Exp_RELOP:
      assert(0);
      break;
    case Exp_PLUS:
      irExp(t->ch[0]);
      irExp(t->ch[2]);
      code(OP_ADD, t->label, t->ch[0]->label, t->ch[2]->label);
      break;
    case Exp_FunCall:
      irExp_FunCall(t);
      break;
    case Exp_Id:
      codes1(OP_LOAD, t->label, IDs[t->ch[0]->int_val], 0);
      break;
    case Exp_Constant:
      code(OP_LOAD_IMM, t->label, t->ch[0]->int_val, 0);
      break;
    default:
      printf("### %d\n", t->int_val);
      assert(0);
  }
}

static void irArgs(Tree *t){
  if(t->ch[2]){ //reverse order
    irArgs(t->ch[2]);
  }
  irExp(t->ch[0]);
  code(OP_ARG, 0, t->ch[0]->label, 0);
}

void irInit(){
  ir_code = vector_new();
}

void irFinish(){
  printIRCode();
}

void irFunc(Tree *t){
  assert(t);
  const char *funName = t->ch[1]->var_name;
  SymTabEntry *e = trieQuery(symTabFunctions, funName);
  codes1(OP_FUNCTION, 0, funName, 0);
  for(ListNode *n = e->paramList->head;n;n = n->next){
    Param *p = n->val;
    codes1(OP_PARAM, 0, p->name, 0);
  }

  assert(t->ch[2]);
  irCompSt(t->ch[2]);
}
