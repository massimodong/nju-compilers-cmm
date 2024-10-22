#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int min(int a, int b){return a < b ? a : b;}

Vector *vector_new();
void vec_pb(Vector *, IRCode);
SymTabEntry *trieQuery(Trie *, const char *);
void irOptimize(Vector *);
void printMIPS(Vector *);

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
  irc.cnst1 = irc.cnst2 = 0;
  vec_pb(ir_code, irc);
}

static void coded(int op, const char *dst_var, int src1, int src2){
  IRCode irc;
  irc.op = op;
  irc.dst_var = dst_var;
  irc.src1 = src1;
  irc.src2 = src2;
  irc.cnst1 = irc.cnst2 = 0;
  vec_pb(ir_code, irc);
}

static void code(int op, int dst, int src1, int src2){
  if(op == OP_LOAD_IMM){
    IRCode irc;
    irc.op = OP_ASSIGN;
    irc.dst = dst;
    irc.src1 = src1;
    irc.src2 = src2;
    irc.cnst1 = 1;
    irc.cnst2 = 0;
    vec_pb(ir_code, irc);
  }else{
    IRCode irc;
    irc.op = op;
    irc.dst = dst;
    irc.src1 = src1;
    irc.src2 = src2;
    irc.cnst1 = irc.cnst2 = 0;
    vec_pb(ir_code, irc);
  }
}

extern List symTabStack;
static int get_var_label(const char *var_name){
  SymTabEntry *entry = trieQuery(symTabStack.rear->val, var_name);
  assert(entry);
  return entry->label;
}

void src1s(IRCode ir){
  if(ir.cnst1){
    fprintf(fir, "#%d", ir.src1);
  }else{
    fprintf(fir, "t%d", ir.src1);
  }
}

void src2s(IRCode ir){
  if(ir.cnst2){
    fprintf(fir, "#%d", ir.src2);
  }else{
    fprintf(fir, "t%d", ir.src2);
  }
}

const char *relop2str(int op){
  switch(op){
    case OP_IFG_GOTO:
      return ">";
    case OP_IFL_GOTO:
      return "<";
    case OP_IFGE_GOTO:
      return ">=";
    case OP_IFLE_GOTO:
      return "<=";
    case OP_IFEQ_GOTO:
      return "==";
    case OP_IFNE_GOTO:
      return "!=";
    default:
      assert(0);
  }
}

const char *art2str(int op){
  switch(op){
    case OP_ADD:
      return "+";
    case OP_SUB:
      return "-";
    case OP_MUL:
      return "*";
    case OP_DIV:
      return "/";
    default:
      assert(0);
  }
}

static void printIR(IRCode ir){
  switch(ir.op){
    case OP_LABEL:
      fprintf(fir, "LABEL Label%d :", ir.src1);
      break;

    case OP_FUNCTION:
      fprintf(fir, "FUNCTION %s :", ir.src1_var);
      break;
    case OP_PARAM:
      fprintf(fir, "PARAM t%d", ir.dst);
      break;
    case OP_FUNCALL:
      fprintf(fir, "t%d := CALL %s", ir.dst, ir.src1_var);
      break;
    case OP_ARG:
      fprintf(fir, "ARG ");
      src1s(ir);
      break;
    case OP_DEC:
      fprintf(fir, "DEC t%d %d", ir.src1, ir.src2);
      break;

    case OP_READ:
      fprintf(fir, "READ t%d", ir.dst);
      break;
    case OP_WRITE:
      fprintf(fir, "WRITE ");
      src1s(ir);
      break;

    case OP_ASSIGN:
      fprintf(fir, "t%d := ", ir.dst);
      src1s(ir);
      break;
    case OP_LOAD_IMM:
      assert(0);
      break;
    case OP_GETADDR:
      fprintf(fir, "t%d := &t%d", ir.dst, ir.src1);
      break;
    case OP_PUTADDR:
      fprintf(fir, "*t%d := ", ir.src2);
      src1s(ir);
      break;
    case OP_GETFROMADDR:
      fprintf(fir, "t%d := *t%d", ir.dst, ir.src1);
      break;

    case OP_GOTO:
      fprintf(fir, "GOTO Label%d", ir.dst);
      break;
    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
      fprintf(fir, "IF ");
      src1s(ir);
      fprintf(fir, " %s ", relop2str(ir.op));
      src2s(ir);
      fprintf(fir, " GOTO Label%d", ir.dst);
      break;

    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      fprintf(fir, "t%d := ", ir.dst);
      src1s(ir);
      fprintf(fir, " %s ", art2str(ir.op));
      src2s(ir);
      break;

    case OP_RETURN:
      fprintf(fir, "RETURN ");
      src1s(ir);
      break;
    default:
      fprintf(stderr, "not implemented!\n");
      assert(0);
  }
  fprintf(fir, "\n");
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
static void irExp(Tree *, int, int, int);
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
      irExp(t->ch[0], 0, 0, -1);
      break;
    case Stmt_CompSt:
      irCompSt(t->ch[0]);
      break;
    case Stmt_Return:
      irExp(t->ch[1], 0, 0, 0);
      code(OP_RETURN, 0, t->ch[1]->label, 0);
      break;
    case Stmt_If:
      if(t->ch[6]){
        int true_label = ++label_cnt, false_label = ++label_cnt, end_label = ++label_cnt;
        irExp(t->ch[2], true_label, false_label, 0);
        code(OP_LABEL, 0, true_label, 0);
        irStmt(t->ch[4]);
        code(OP_GOTO, end_label, 0, 0);
        code(OP_LABEL, 0, false_label, 0);
        irStmt(t->ch[6]);
        code(OP_LABEL, 0, end_label, 0);
      }else{
        int true_label = ++label_cnt, end_label = ++label_cnt;
        irExp(t->ch[2], true_label, end_label, 0);
        code(OP_LABEL, 0, true_label, 0);
        irStmt(t->ch[4]);
        code(OP_LABEL, 0, end_label, 0);
      }
      break;
    case Stmt_While: {
      int start_label = ++label_cnt, true_label = ++label_cnt, end_label = ++label_cnt;
      code(OP_LABEL, 0, start_label, 0);
      irExp(t->ch[2], true_label, end_label, 0);
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
    irExp(t->ch[2], 0, 0, get_var_label(name));
    //code(OP_ASSIGN, get_var_label(name), t->ch[2]->label, 0);
  }
}

static void irAnd(Tree *t,int tl, int fl, int out){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
    assert(out == 0);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }
  int ml = ++label_cnt;

  if(out == -1){
    irExp(t->ch[0], ml, fl, 0);
    code(OP_LABEL, 0, ml, 0);
    irExp(t->ch[2], 0, 0, -1);
    code(OP_LABEL, 0, fl, 0);
    return;
  }

  irExp(t->ch[0], ml, fl, 0);
  code(OP_LABEL, 0, ml, 0);
  irExp(t->ch[2], tl, fl, 0);

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

static void irOr(Tree *t, int tl, int fl, int out){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }
  int ml = ++label_cnt;

  if(out == -1){
    irExp(t->ch[0], tl, ml, 0);
    code(OP_LABEL, 0, ml, 0);
    irExp(t->ch[2], 0, 0, -1);
    code(OP_LABEL, 0, tl, 0);
    return;
  }

  irExp(t->ch[0], tl, ml, 0);
  code(OP_LABEL, 0, ml, 0);
  irExp(t->ch[2], tl, fl, 0);

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

static void irRelop(Tree *t, int tl, int fl, int out){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
    assert(out == 0);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }

  if(out == -1){
    irExp(t->ch[0], 0, 0, -1);
    irExp(t->ch[2], 0, 0, -1);
    return;
  }

  irExp(t->ch[0], 0, 0, 0);
  irExp(t->ch[2], 0, 0, 0);
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

static void irExp_FunCall(Tree *t, int out){
  const char *funName = IDs[t->ch[0]->int_val];

  if(strcmp(funName, "read") == 0){
    if(out != -1) code(OP_READ, t->label, 0, 0);
    else code(OP_READ, ++label_cnt, 0, 0);
    return;
  }else if(strcmp(funName, "write") == 0){
    assert(t->ch[2]);
    irExp(t->ch[2]->ch[0], 0, 0, 0);
    code(OP_WRITE, 0, t->ch[2]->ch[0]->label, 0);
    if(out != -1) code(OP_LOAD_IMM, t->label, 0, 0);
    return;
  }

  if(out != -1){
    if(t->ch[2]){
      irArgs(t->ch[2]);
    }
    codes1(OP_FUNCALL, t->label, funName, 0);
  }else{
    if(t->ch[2]){
      irArgs(t->ch[2]);
    }
    codes1(OP_FUNCALL, ++label_cnt, funName, 0);
  }
}

static void irNot(Tree *t, int tl, int fl, int out){
  int generate_labels = 0;
  if(tl || fl){
    assert(tl && fl);
    assert(out == 0);
  }else{
    tl = ++label_cnt;
    fl = ++label_cnt;
    generate_labels = 1;
  }

  if(out == -1){
    irExp(t->ch[1], 0, 0, -1);
    return;
  }

  irExp(t->ch[1], fl, tl, 0);

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

static int irExpGetAddr(Tree *t){
  if(t->int_val == Exp_QueryArray){
    int ol = irExpGetAddr(t->ch[0]);

    int pace_label = ++label_cnt;
    Type *type = t->ch[0]->exp_type;
    code(OP_LOAD_IMM, pace_label, type->next->totsize, 0);

    int offset_label = ++label_cnt;
    irExp(t->ch[2], 0, 0, 0);
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
    irExp(t, 0, 0, 0); //TODO
    code(OP_ASSIGN, ++label_cnt, t->label, 0);
    return label_cnt;
  }else{
    assert(0);
  }
}

static void irExpAssign(Tree *t, int out){
  if(t->ch[0]->int_val == Exp_Id && t->ch[0]->exp_type->type == 0){
    t->label = get_var_label(IDs[t->ch[0]->ch[0]->int_val]);
    irExp(t->ch[2], 0, 0, t->label);
    if(out > 0) code(OP_ASSIGN, out, t->label, 0);
  }else if(t->ch[0]->exp_type->type == 0){
    irExp(t->ch[2], 0, 0, 0);
    int addr_label = irExpGetAddr(t->ch[0]);
    code(OP_PUTADDR, 0, t->ch[2]->label, addr_label);
    t->label = t->ch[2]->label;
    if(out > 0) code(OP_ASSIGN, out, t->label, 0);
  }else{
    int l_addr_label = irExpGetAddr(t->ch[0]),
        r_addr_label = irExpGetAddr(t->ch[2]);

    int size = min(t->ch[0]->exp_type->totsize, t->ch[2]->exp_type->totsize);
    int four_label = ++label_cnt;
    //code(OP_ASSIGN, t->label, l_addr_label, 0);
    t->label = l_addr_label;
    assert(out <= 0);
    code(OP_LOAD_IMM, four_label, 4, 0);
    for(int i=0;i<size;i+=4){
      int t_label = ++label_cnt;
      code(OP_GETFROMADDR, t_label, r_addr_label, 0);
      code(OP_PUTADDR, 0, t_label, l_addr_label);

      int nl = ++label_cnt, nr = ++label_cnt;
      code(OP_ADD, nl, l_addr_label, four_label);
      code(OP_ADD, nr, r_addr_label, four_label);
      l_addr_label = nl;
      r_addr_label = nr;
    }
  }
}

/*
 * resolve an expression
 * params:
 *  `t`           The tree node of the expression
 *  `true_label`  valid if non-zero, GOTO to this label when exp evaluating to non-zero
 *  `false_label` valid if non-zero, GOTO to this label when exp evaluating to zero
 *  `out`         valid if `true_label` and `false_label` are not set.
 *                if `out` == -1, we don't care the value of this expression;
 *                if `out` == 0, we should evaluate this exp to a new label
 *                if `out` > 0, we should evaluate this exp to label #`out`
 */
static void irExp(Tree *t, int true_label, int false_label, int out){
  int addr_label;
  if(out == 0){
    t->label = ++label_cnt;
  }else if(out > 0){
    t->label = out;
  }else{
    t->label = -0x130BC99;
  }
  switch(t->int_val){
    case Exp_ASSIGN:
      irExpAssign(t, out);
      break;
    case Exp_AND:
      irAnd(t, true_label, false_label, out);
      return; //Attention: We RETURN Here
    case Exp_OR:
      irOr(t, true_label, false_label, out);
      return; //Attention: We RETURN Here
    case Exp_RELOP:
      irRelop(t, true_label, false_label, out);
      return; //Attention: We RETURN Here
    case Exp_PLUS:
    case Exp_MINUS:
    case Exp_STAR:
    case Exp_DIV:
      if(out == -1){
        irExp(t->ch[0], 0, 0, -1);
        irExp(t->ch[2], 0, 0, -1);
        return; //Attention: We RETURN Here
      }
      irExp(t->ch[0], 0, 0, 0);
      irExp(t->ch[2], 0, 0, 0);
      if(t->int_val == Exp_PLUS) code(OP_ADD, t->label, t->ch[0]->label, t->ch[2]->label);
      else if(t->int_val == Exp_MINUS) code(OP_SUB, t->label, t->ch[0]->label, t->ch[2]->label);
      else if(t->int_val == Exp_STAR) code(OP_MUL, t->label, t->ch[0]->label, t->ch[2]->label);
      else if(t->int_val == Exp_DIV) code(OP_DIV, t->label, t->ch[0]->label, t->ch[2]->label);
      else assert(0);
      break;
    case Exp_Parentheses:
      irExp(t->ch[1], true_label, false_label, out);
      t->label = t->ch[1]->label;
      return; //Attention: We RETURN Here
    case Exp_NEG:
      if(out == -1){
        irExp(t->ch[1], 0, 0, -1);
        return; //Attention: We RETURN Here
      }
      irExp(t->ch[1], 0, 0, 0);
      code(OP_LOAD_IMM, ++label_cnt, 0, 0);
      code(OP_SUB, t->label, label_cnt, t->ch[1]->label);
      break;
    case Exp_NOT:
      irNot(t, true_label, false_label, out);
      break;
    case Exp_FunCall:
      irExp_FunCall(t, out);
      break;
    case Exp_QueryArray:
    case Exp_QueryStruct:
      //assert(typeEq(t->type, IntType)); //TODO: should not assert
      addr_label = irExpGetAddr(t);
      if(out != -1) code(OP_GETFROMADDR, t->label, addr_label, 0);
      break;
    case Exp_Id:
      t->label = get_var_label(IDs[t->ch[0]->int_val]);
      if(out > 0) code(OP_ASSIGN, out, t->label, 0);
      //code(OP_ASSIGN, t->label, get_var_label(IDs[t->ch[0]->int_val]), 0);
      break;
    case Exp_Constant:
      if(out != -1) code(OP_LOAD_IMM, t->label, t->ch[0]->int_val, 0);
      break;
    default:
      printf("### %d\n", t->int_val);
      assert(0);
  }
  if(true_label){
    assert(false_label);
    code(OP_LOAD_IMM, ++label_cnt, 0, 0);
    code(OP_IFNE_GOTO, true_label, t->label, label_cnt);
    code(OP_GOTO, false_label, 0, 0);
  }
}

static void irArgs(Tree *t){
  int addr_label;

  if(t->ch[2]){ //reverse order
    irArgs(t->ch[2]);
  }

  if(t->ch[0]->exp_type->type == 0){
    irExp(t->ch[0], 0, 0, 0);
    code(OP_ARG, 0, t->ch[0]->label, 0);
  }else{
    addr_label = irExpGetAddr(t->ch[0]);
    code(OP_ARG, 0, addr_label, 0);
  }

  /*
  if(t->ch[0]->exp_type->type == 0){
    code(OP_ARG, 0, t->ch[0]->label, 0);
  }else{
    code(OP_ARG, 0, addr_label, 0);
  }
  */

}

void irInit(){
  ir_code = vector_new();
}

void irFinish(){
  irOptimize(ir_code);
  //printIRCode();
  printMIPS(ir_code);
}

void irFunc(Tree *t){
  assert(t);
  const char *funName = t->ch[1]->var_name;
  SymTabEntry *e = trieQuery(symTabFunctions, funName);
  codes1(OP_FUNCTION, 0, funName, 0);
  for(ListNode *n = e->paramList->head;n;n = n->next){
    Param *p = n->val;
    code(OP_PARAM, get_var_label(p->name), 0, 0);
  }

  assert(t->ch[2]);
  irCompSt(t->ch[2]);
}
