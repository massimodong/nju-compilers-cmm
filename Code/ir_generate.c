#include "common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int min(int a, int b){return a < b ? a : b;}

Vector *vector_new();
void vec_pb(Vector *, IRCode);
SymTabEntry *trieQuery(Trie *, const char *);

extern Trie *symTabFunctions;
extern FILE *fir;
extern char **IDs;

static Vector *ir_code;
int label_cnt = 0;

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

extern List symTabStack;
static int get_var_label(const char *var_name){
  SymTabEntry *entry = trieQuery(symTabStack.rear->val, var_name);
  assert(entry);
  return entry->label;
}

static void printIR(IRCode ir){
  switch(ir.op){
    case OP_LABEL:
      fprintf(fir, "LABEL Label%d :\n", ir.src1);
      break;

    case OP_FUNCTION:
      fprintf(fir, "FUNCTION %s :\n", ir.src1_var);
      break;
    case OP_PARAM:
      fprintf(fir, "PARAM t%d\n", ir.src1);
      break;
    case OP_FUNCALL:
      fprintf(fir, "t%d := CALL %s\n", ir.dst, ir.src1_var);
      break;
    case OP_ARG:
      fprintf(fir, "ARG t%d\n", ir.src1);
      break;
    case OP_DEC:
      fprintf(fir, "DEC t%d %d\n", ir.src1, ir.src2);
      break;

    case OP_READ:
      fprintf(fir, "READ t%d\n", ir.dst);
      break;
    case OP_WRITE:
      fprintf(fir, "WRITE t%d\n", ir.src1);
      break;

    case OP_ASSIGN:
      fprintf(fir, "t%d := t%d\n", ir.dst, ir.src1);
      break;
    case OP_LOAD_IMM:
      fprintf(fir, "t%d := #%d\n", ir.dst, ir.src1);
      break;
    case OP_GETADDR:
      fprintf(fir, "t%d := &t%d\n", ir.dst, ir.src1);
      break;
    case OP_PUTADDR:
      fprintf(fir, "*t%d := t%d\n", ir.dst, ir.src1);
      break;
    case OP_GETFROMADDR:
      fprintf(fir, "t%d := *t%d\n", ir.dst, ir.src1);
      break;

    case OP_GOTO:
      fprintf(fir, "GOTO Label%d\n", ir.dst);
      break;
    case OP_IFG_GOTO:
      fprintf(fir, "IF t%d > t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;
    case OP_IFL_GOTO:
      fprintf(fir, "IF t%d < t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;
    case OP_IFGE_GOTO:
      fprintf(fir, "IF t%d >= t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;
    case OP_IFLE_GOTO:
      fprintf(fir, "IF t%d <= t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;
    case OP_IFEQ_GOTO:
      fprintf(fir, "IF t%d == t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;
    case OP_IFNE_GOTO:
      fprintf(fir, "IF t%d != t%d GOTO Label%d\n", ir.src1, ir.src2, ir.dst);
      break;

    case OP_ADD:
      fprintf(fir, "t%d := t%d + t%d\n", ir.dst, ir.src1, ir.src2);
      break;
    case OP_SUB:
      fprintf(fir, "t%d := t%d - t%d\n", ir.dst, ir.src1, ir.src2);
      break;
    case OP_MUL:
      fprintf(fir, "t%d := t%d * t%d\n", ir.dst, ir.src1, ir.src2);
      break;
    case OP_DIV:
      fprintf(fir, "t%d := t%d / t%d\n", ir.dst, ir.src1, ir.src2);
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
static void irDec(Tree *);
static void irExp(Tree *, int, int); //syn: label
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
      irExp(t->ch[0], 0, 0);
      break;
    case Stmt_CompSt:
      irCompSt(t->ch[0]);
      break;
    case Stmt_Return:
      irExp(t->ch[1], 0, 0);
      code(OP_RETURN, 0, t->ch[1]->label, 0);
      break;
    case Stmt_If:
      if(t->ch[6]){
        int true_label = ++label_cnt, false_label = ++label_cnt, end_label = ++label_cnt;
        irExp(t->ch[2], true_label, false_label);
        code(OP_LABEL, 0, true_label, 0);
        irStmt(t->ch[4]);
        code(OP_GOTO, end_label, 0, 0);
        code(OP_LABEL, 0, false_label, 0);
        irStmt(t->ch[6]);
        code(OP_LABEL, 0, end_label, 0);
      }else{
        int true_label = ++label_cnt, end_label = ++label_cnt;
        irExp(t->ch[2], true_label, end_label);
        code(OP_LABEL, 0, true_label, 0);
        irStmt(t->ch[4]);
        code(OP_LABEL, 0, end_label, 0);
      }
      break;
    case Stmt_While: {
      int start_label = ++label_cnt, true_label = ++label_cnt, end_label = ++label_cnt;
      code(OP_LABEL, 0, start_label, 0);
      irExp(t->ch[2], true_label, end_label);
      code(OP_LABEL, 0, true_label, 0);
      irStmt(t->ch[4]);
      code(OP_GOTO, start_label, 0, 0);
      code(OP_LABEL, 0, end_label, 0);
    }
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
  irDec(t->ch[0]);
  if(t->ch[2]) irDecList(t->ch[2]);
}

static void irDec(Tree *t){
  const char *name = t->ch[0]->var_name;
  Type *type = t->ch[0]->exp_type;

  if(type->type == 2 || type->type == 3){
    code(OP_DEC, 0, ++label_cnt, type->totsize);
    code(OP_GETADDR, get_var_label(name), label_cnt, 0);
  }

  if(t->ch[2]){ //initialize
    irExp(t->ch[2], 0, 0);
    code(OP_ASSIGN, get_var_label(name), t->ch[2]->label, 0);
  }
}

static void irAnd(Tree *t,int tl, int fl){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }
  int ml = ++label_cnt;
  irExp(t->ch[0], ml, fl);
  code(OP_LABEL, 0, ml, 0);
  irExp(t->ch[2], tl, fl);

  if(generate_labels){
    int el = ++label_cnt;
    code(OP_LABEL, 0, tl, 0);
    code(OP_LOAD_IMM, t->label, 1, 0);
    code(OP_GOTO, el, 0, 0);
    code(OP_LABEL, 0, fl, 0);
    code(OP_LOAD_IMM, t->label, 0 , 0);
    code(OP_LABEL, 0, el, 0);
  }
}

static void irOr(Tree *t, int tl, int fl){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }
  int ml = ++label_cnt;
  irExp(t->ch[0], tl, ml);
  code(OP_LABEL, 0, ml, 0);
  irExp(t->ch[2], tl, fl);

  if(generate_labels){
    int el = ++label_cnt;
    code(OP_LABEL, 0, tl, 0);
    code(OP_LOAD_IMM, t->label, 1, 0);
    code(OP_GOTO, el, 0, 0);
    code(OP_LABEL, 0, fl, 0);
    code(OP_LOAD_IMM, t->label, 0 , 0);
    code(OP_LABEL, 0, el, 0);
  }
}

static void irRelop(Tree *t, int tl, int fl){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }
  irExp(t->ch[0], 0, 0);
  irExp(t->ch[2], 0, 0);
  switch(t->ch[1]->int_val){
    case REL_G:
      code(OP_IFG_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
    case REL_L:
      code(OP_IFL_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
    case REL_GE:
      code(OP_IFGE_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
    case REL_LE:
      code(OP_IFLE_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
    case REL_EQ:
      code(OP_IFEQ_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
    case REL_NE:
      code(OP_IFNE_GOTO, tl, t->ch[0]->label, t->ch[2]->label);
      break;
  }
  code(OP_GOTO, fl, 0, 0);

  if(generate_labels){
    int el = ++label_cnt;
    code(OP_LABEL, 0, fl, 0);
    code(OP_LOAD_IMM, t->label, 0, 0);
    code(OP_GOTO, el, 0, 0);
    code(OP_LABEL, 0, tl, 0);
    code(OP_LOAD_IMM, t->label, 1, 0);
    code(OP_LABEL, 0, el, 0);
  }
}

static void irExp_FunCall(Tree *t){
  const char *funName = IDs[t->ch[0]->int_val];

  if(strcmp(funName, "read") == 0){
    code(OP_READ, t->label, 0, 0);
    return;
  }else if(strcmp(funName, "write") == 0){
    assert(t->ch[2]);
    irExp(t->ch[2]->ch[0], 0, 0);
    code(OP_WRITE, 0, t->ch[2]->ch[0]->label, 0);
    code(OP_LOAD_IMM, t->label, 0, 0);
    return;
  }

  if(t->ch[2]){
    irArgs(t->ch[2]);
  }

  codes1(OP_FUNCALL, t->label, funName, 0);
}

static void irNot(Tree *t){
  irExp(t->ch[1], 0, 0);
  int zero_label = ++label_cnt, false_label = ++label_cnt, end_label = ++label_cnt;
  code(OP_LOAD_IMM, zero_label, 0, 0);
  code(OP_IFEQ_GOTO, false_label, t->ch[1]->label, zero_label);
  code(OP_LOAD_IMM, t->label, 0, 0);
  code(OP_GOTO, end_label, 0, 0);
  code(OP_LABEL, 0, false_label, 0);
  code(OP_LOAD_IMM, t->label, 1, 0);
  code(OP_LABEL, 0, end_label, 0);
}

static int irExpGetAddr(Tree *t){
  if(t->int_val == Exp_QueryArray){
    int ol = irExpGetAddr(t->ch[0]);

    int pace_label = ++label_cnt;
    Type *type = t->ch[0]->exp_type;
    code(OP_LOAD_IMM, pace_label, type->next->totsize, 0);

    int offset_label = ++label_cnt;
    irExp(t->ch[2], 0, 0);
    code(OP_MUL, offset_label, pace_label, t->ch[2]->label);

    code(OP_ADD, ++label_cnt, ol, offset_label);
    return label_cnt;
  }else if(t->int_val == Exp_QueryStruct){
    int ol = irExpGetAddr(t->ch[0]);
    const char *name = IDs[t->ch[2]->int_val];
    Type *type = t->ch[0]->exp_type;
    SymTabEntry *entry = trieQuery(type->map, name);

    int offset_label = ++label_cnt;
    code(OP_LOAD_IMM, offset_label, entry->structEntry->offset, 0);
    code(OP_ADD, ++label_cnt, ol, offset_label);
    return label_cnt;
  }else if(t->int_val == Exp_Id){
    code(OP_ASSIGN, ++label_cnt, get_var_label(IDs[t->ch[0]->int_val]), 0);
    return label_cnt;
  }else if(t->int_val == Exp_Parentheses){
    return irExpGetAddr(t->ch[1]);
  }else if(t->int_val == Exp_ASSIGN){ // resolve Assign Exp, which returns label of l_addr
    irExp(t, 0, 0);
    code(OP_ASSIGN, ++label_cnt, t->label, 0);
    return label_cnt;
  }else{
    assert(0);
  }
}

static void irExpAssign(Tree *t){
  if(t->ch[0]->int_val == Exp_Id && t->ch[0]->exp_type->type == 0){
    irExp(t->ch[2], 0, 0);
    code(OP_ASSIGN, get_var_label(IDs[t->ch[0]->ch[0]->int_val]), t->ch[2]->label, 0);
    t->label = get_var_label(IDs[t->ch[0]->ch[0]->int_val]);
  }else if(t->ch[0]->exp_type->type == 0){
    irExp(t->ch[2], 0, 0);
    int addr_label = irExpGetAddr(t->ch[0]);
    code(OP_PUTADDR, addr_label, t->ch[2]->label, 0);
    t->label = t->ch[2]->label;
  }else{
    int l_addr_label = irExpGetAddr(t->ch[0]),
        r_addr_label = irExpGetAddr(t->ch[2]);

    int size = min(t->ch[0]->exp_type->totsize, t->ch[2]->exp_type->totsize);
    int t_label = ++label_cnt, four_label = ++label_cnt;
    code(OP_ASSIGN, t->label, l_addr_label, 0);
    code(OP_LOAD_IMM, four_label, 4, 0);
    for(int i=0;i<size;i+=4){
      code(OP_GETFROMADDR, t_label, r_addr_label, 0);
      code(OP_PUTADDR, l_addr_label, t_label, 0);
      code(OP_ADD, l_addr_label, l_addr_label, four_label);
      code(OP_ADD, r_addr_label, r_addr_label, four_label);
    }
  }
  /*
  switch(t->ch[0]->int_val){
    case Exp_Id:
      code(OP_ASSIGN, get_var_label(IDs[t->ch[0]->ch[0]->int_val]), t->ch[2]->label, 0);
      break;
    default:
      addr_label = irExpGetAddr(t->ch[0]);
      code(OP_PUTADDR, addr_label, t->ch[2]->label, 0);
      break;
  }
  */
}

static void irExp(Tree *t, int true_label, int false_label){
  int addr_label;
  t->label = ++label_cnt;
  switch(t->int_val){
    case Exp_ASSIGN:
      irExpAssign(t);
      break;
    case Exp_AND:
      irAnd(t, true_label, false_label);
      return; //Attention: We RETURN Here
    case Exp_OR:
      irOr(t, true_label, false_label);
      return; //Attention: We RETURN Here
    case Exp_RELOP:
      irRelop(t, true_label, false_label);
      return; //Attention: We RETURN Here
    case Exp_PLUS:
      irExp(t->ch[0], 0, 0);
      irExp(t->ch[2], 0, 0);
      code(OP_ADD, t->label, t->ch[0]->label, t->ch[2]->label);
      break;
    case Exp_MINUS:
      irExp(t->ch[0], 0, 0);
      irExp(t->ch[2], 0, 0);
      code(OP_SUB, t->label, t->ch[0]->label, t->ch[2]->label);
      break;
    case Exp_STAR:
      irExp(t->ch[0], 0, 0);
      irExp(t->ch[2], 0, 0);
      code(OP_MUL, t->label, t->ch[0]->label, t->ch[2]->label);
      break;
    case Exp_DIV:
      irExp(t->ch[0], 0, 0);
      irExp(t->ch[2], 0, 0);
      code(OP_DIV, t->label, t->ch[0]->label, t->ch[2]->label);
      break;
    case Exp_Parentheses:
      irExp(t->ch[1], 0, 0);
      t->label = t->ch[1]->label;
      break;
    case Exp_NEG:
      irExp(t->ch[1], 0, 0);
      code(OP_LOAD_IMM, ++label_cnt, 0, 0);
      code(OP_SUB, t->label, label_cnt, t->ch[1]->label);
      break;
    case Exp_NOT:
      irNot(t);
      break;
    case Exp_FunCall:
      irExp_FunCall(t);
      break;
    case Exp_QueryArray:
    case Exp_QueryStruct:
      //assert(typeEq(t->type, IntType)); //TODO: should not assert
      addr_label = irExpGetAddr(t);
      code(OP_GETFROMADDR, t->label, addr_label, 0);
      break;
    case Exp_Id:
      code(OP_ASSIGN, t->label, get_var_label(IDs[t->ch[0]->int_val]), 0);
      break;
    case Exp_Constant:
      code(OP_LOAD_IMM, t->label, t->ch[0]->int_val, 0);
      break;
    default:
      printf("### %d\n", t->int_val);
      assert(0);
  }
  if(true_label){
    code(OP_LOAD_IMM, ++label_cnt, 0, 0);
    code(OP_IFNE_GOTO, true_label, t->label, label_cnt);
  }
  if(false_label){
    code(OP_LOAD_IMM, ++label_cnt, 0, 0);
    code(OP_IFEQ_GOTO, false_label, t->label, label_cnt);
  }
}

static void irArgs(Tree *t){
  if(t->ch[2]){ //reverse order
    irArgs(t->ch[2]);
  }
  if(t->ch[0]->exp_type->type == 0){
    irExp(t->ch[0], 0, 0);
    code(OP_ARG, 0, t->ch[0]->label, 0);
  }else{
    int addr_label = irExpGetAddr(t->ch[0]);
    code(OP_ARG, 0, addr_label, 0);
  }
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
    code(OP_PARAM, 0, get_var_label(p->name), 0);
  }

  assert(t->ch[2]);
  irCompSt(t->ch[2]);
}
