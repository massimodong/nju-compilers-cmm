#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "syntax.tab.h"

extern char **IDs;
int trieInsert(Trie **, const char *, SymTabEntry *);
SymTabEntry *trieQuery(Trie *, const char *);
void listAppend(List *, void *);
void listPopRear(List *);
List *listMerge(List *, List *);
List *newList();

Type *IntType, *FloatType;

Type *newType(){
  return malloc(sizeof(Type));
}
Type *makeArray(Type *t, int n){
  Type *ret = newType();
  ret->type = 2;
  ret->size = n;
  ret->next = t;
  if(t->type == 2) ret->last = t->last;
  else ret->last = t;
  return ret;
}
Type *makeStruct(Type *t, const char *name){
  assert(t);
  Type *ret = newType();
  ret->type = 3;
  ret->name = name;
  ret->structType = t;
  ret->next = NULL;
  ret->last = ret;
  return ret;
}
Type *mergeStruct(Type *t, Type *tr){
  if(t == NULL) return tr;
  else if(tr == NULL) return t;
  assert(t->type == 3);
  assert(tr->type == 3);
  assert(t->structType);
  assert(tr->structType);
  t->last->next = tr;
  t->last = tr->last;
  return t;
}
int structConstructMap(Type *t){
  t->map = NULL;
  int ok = 1;
  for(Type *n = t;n;n=n->next){
    SymTabEntry *entry = malloc(sizeof(SymTabEntry));
    entry->name = n->name;
    entry->depth = 0;
    entry->isStructDec = 0;
    entry->type = n;
    if(!trieInsert(&t->map, n->name, entry)){
      ok = 0;
    }
  }
  return ok;
}
List *makeParam(const char *name, Type *type){
  Param *param = malloc(sizeof(Param));
  param->type = type;
  param->name = name;
  List *list = newList();
  listAppend(list, param);
  return list;
}

int typeEq(Type *t1, Type *t2){
  if(t1->type == 0) return t2->type == 0;
  else if(t1->type == 1) return t2->type == 1;
  else if(t1->type == 2){
    if(t2->type != 2) return 0;
    return typeEq(t1->next, t2->next);
  }else if(t1->type == 3){
    if(t2->type != 3) return 0;
    assert(0);
  }else{
    assert(0);
  }
}

void printType(Type *t){
  if(t == NULL) printf("<undefined>");
  else if(t->type == 0) printf("int");
  else if(t->type == 1) printf("float");
  else if(t->type == 2){
    printf("array(%d, ", t->size);
    printType(t->next);
    printf(")");
  }else{
    assert(t->type == 3);
    printf("struct{");
    for(Type *p = t; p; p = p->next){
      printf("(%s: ", p->name);
      assert(p->structType);
      printType(p->structType);
      printf(")");
    }
    printf("}");
  }
}

List symTabStack = (List){NULL, NULL};
int symTabStackDepth = 0;
void symTabStackPush(){
  listAppend(&symTabStack, symTabStack.rear->val);
  ++symTabStackDepth;
}
void symTabStackPop(){
  --symTabStackDepth;
  listPopRear(&symTabStack);
}

/* register a variable, return whether success
 */
int registerVariable(const char *name, Type *type, int isStructDec){
  if(isStructDec){
    printf("register struct %s as: ", name);
    printType(type);
    printf("\n");
  }else{
    printf("register variable %s of type: ", name);
    printType(type);
    printf("\n");
  }

  SymTabEntry *entry= malloc(sizeof(SymTabEntry));
  entry->name = name;
  entry->depth = symTabStackDepth;
  entry->isStructDec = isStructDec;
  entry->type = type;

  return trieInsert(&symTabStack.rear->val, name, entry);
}

Trie *symTabFunctions;
SymTabEntry *curFunction;
int registerFunction(const char *name, Type *retType, List *paramList){
  printf("register function ");
  printType(retType);
  printf(" <%s>(", name);
  for(ListNode *n = paramList->head; n; n=n->next){
    Param *p = n->val;
    printType(p->type);
    printf(" %s,", p->name);
  }
  printf(")\n");
  SymTabEntry *entry = malloc(sizeof(SymTabEntry));
  entry->name = name;
  entry->depth = 0;
  entry->returnType = retType;
  entry->paramList = paramList;
  entry->defined = 0;

  return trieInsert(&symTabFunctions, name, entry);
}


void sdd_error(int n, const char *msg, Tree *t){
  printf("Error type %d at Line %d: %s.\n", n, t->lineno, msg);
}

void resolveProgram(Tree *);
void resolveExtDefList(Tree *);
void resolveExtDef(Tree *);
void resolveExtDecList(Tree *); //inh: type; registers variables
void resolveSpecifier(Tree *); //syn: type;
void resolveStructSpecifier(Tree *); //syn: type; registers struct name if exists
//OptTag and Tag, ommited
void resolveVarDec(Tree *); //inh: type; syn: type and name
void resolveFunDec(Tree *); //inh: type; syn: name; registers function
void resolveVarList(Tree *); //syn: paramList;
void resolveParamDec(Tree *); //syn: paramList (of size 1)
void resolveCompSt(Tree *); //push before, pop after
void resolveStmtList(Tree *);
void resolveStmt(Tree *);

void resolveDefList_fromCompSt(Tree *);
void resolveDef_fromCompSt(Tree *);
void resolveDecList_fromCompSt(Tree *); //inh: type
void resolveDec_fromCompSt(Tree *); //inh: type; registers variables

void resolveDefList_fromStruct(Tree *); //syn: type
void resolveDef_fromStruct(Tree *); //syn: type
void resolveDecList_fromStruct(Tree *); //inh: type; syn: type (struct to be merged)
void resolveDec_fromStruct(Tree *); //inh: type; syn: type (struct of size 1)

void resolveExp(Tree *); //syn: exp_type
void resolveArgs(Tree *); //syn: arg_list

void resolveProgram(Tree *t){
  IntType = newType();
  FloatType = newType();
  IntType->type = 0;
  FloatType->type = 1;

  listAppend(&symTabStack, NULL);
  symTabStackDepth = 1;

  resolveExtDefList(t->ch[0]);
}

void resolveExtDefList(Tree *t){
  if(t->ch[0]){
    resolveExtDef(t->ch[0]);
    resolveExtDefList(t->ch[1]);
  }
}

void resolveExtDef(Tree *t){
  resolveSpecifier(t->ch[0]);
  switch(t->int_val){
    case ExtDef_Val:
      if(t->ch[1]){ //if has ExtDecList
        t->ch[1]->exp_type = t->ch[0]->exp_type;
        resolveExtDecList(t->ch[1]);
      }
      break;
    case ExtDef_Func:
      t->ch[1]->exp_type = t->ch[0]->exp_type;
      symTabStackPush();
      resolveFunDec(t->ch[1]);
      if(t->ch[2]){ //if has function body
        SymTabEntry *e = trieQuery(symTabFunctions, t->ch[1]->var_name);
        assert(e);
        if(e->defined){
          sdd_error(4, "duplicate definition of function", t);
        }else{
          e->defined = 1;
        }

        curFunction = e;
        resolveCompSt(t->ch[2]);
      }
      symTabStackPop();
      break;
  }
}

void resolveExtDecList(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);
  t->exp_type = t->ch[0]->exp_type;
  t->var_name = t->ch[0]->var_name;
  if(!registerVariable(t->var_name, t->exp_type, 0)){
    sdd_error(3, "TODO", t);
  }
  if(t->ch[2]){
    t->ch[2]->exp_type = t->exp_type;
    resolveExtDecList(t->ch[2]);
  }
}

void resolveSpecifier(Tree *t){
  if(t->int_val == Specifier_Type){ //int or float
    if(t->ch[0]->int_val == 0) t->exp_type = IntType;
    else if(t->ch[0]->int_val == 1) t->exp_type = FloatType;
    else assert(0);
  }else{
    assert(t->int_val == Specifier_Struct);
    resolveStructSpecifier(t->ch[0]);
    t->exp_type = t->ch[0]->exp_type;
  }
}

void resolveStructSpecifier(Tree *t){
    Type *type = NULL;
    if(t->ch[3]){ //if defined body
      symTabStackPush();
      resolveDefList_fromStruct(t->ch[3]);
      symTabStackPop();
      type = t->ch[3]->exp_type;
      if(!structConstructMap(type)){
        sdd_error(15, "duplicate names in struct", t);
      }
    }
    if(t->ch[1]->show){ //Tag name is not empty
      const char *name = IDs[t->ch[1]->ch[0]->int_val];
      if(type){ //if struct has body
        if(!registerVariable(name, type, 1)){
          sdd_error(16, "TODO", t);
        }
      }else{
        SymTabEntry *e = trieQuery(symTabStack.rear->val, name);
        if(e){
          if(e->isStructDec) type = e->type;
          else{
            type = IntType;
            sdd_error(16, "TODO", t);
          }
        }else{
          type = IntType;
          sdd_error(17, "struct not defined", t);
        }
      }
    }
    t->exp_type = type;
}

void resolveVarDec(Tree *t){
  if(t->int_val == VarDec_Id){
    t->var_name = IDs[t->ch[0]->int_val];
  }else{
    t->ch[0]->exp_type = makeArray(t->exp_type, t->ch[2]->int_val);
    resolveVarDec(t->ch[0]);
    t->exp_type = t->ch[0]->exp_type;
    t->var_name = t->ch[0]->var_name;
  }
}

int paramListEq(List *a, List *b){
    for(ListNode *na = a->head, *nb = b->head;na || nb;na=na->next, nb=nb->next){
      if(na == NULL || nb == NULL) return 0;
      Param *pa = na->val, *pb = nb->val;
      if(!typeEq(pa->type, pb->type)) return 0;
    }
    return 1;
}
void resolveFunDec(Tree *t){
  List *paramList = NULL;
  t->var_name = IDs[t->ch[0]->int_val];
  if(t->ch[2]){
    resolveVarList(t->ch[2]);
    paramList = t->ch[2]->var_list;
  }else{
    paramList = newList();
  }

  SymTabEntry *funEntry = trieQuery(symTabFunctions, t->var_name);
  if(funEntry == NULL){
    registerFunction(t->var_name, t->exp_type, paramList);
  }else{
    //check if two function declarations are equivalent
    if(!(typeEq(t->exp_type, funEntry->returnType) && paramListEq(paramList, funEntry->paramList))){
      sdd_error(19, "contradicting function declarations", t);
    }
    //switch to new declaration to match new function body
    funEntry->returnType = t->exp_type;
    funEntry->paramList = paramList; //TODO: maybe we could delete memory of old param list
  }
}

void resolveVarList(Tree *t){
  resolveParamDec(t->ch[0]);
  if(t->ch[2]){
    resolveVarList(t->ch[2]);
    t->var_list = listMerge(t->ch[0]->var_list, t->ch[2]->var_list);
  }else{
    t->var_list = t->ch[0]->var_list;
  }
}

void resolveParamDec(Tree *t){
  resolveSpecifier(t->ch[0]);
  t->ch[1]->exp_type = t->ch[0]->exp_type;
  resolveVarDec(t->ch[1]);
  t->var_list = makeParam(t->ch[1]->var_name, t->ch[1]->exp_type);
}

void resolveCompSt(Tree *t){
  resolveDefList_fromCompSt(t->ch[1]);
  resolveStmtList(t->ch[2]);
}

void resolveStmtList(Tree *t){
  if(t->ch[0]){
    resolveStmt(t->ch[0]);
    resolveStmtList(t->ch[1]);
  }
}

void resolveStmt(Tree *t){
  switch(t->int_val){
    case Stmt_Exp:
      resolveExp(t->ch[0]);
      break;
    case Stmt_CompSt:
      symTabStackPush();
      resolveCompSt(t->ch[0]);
      symTabStackPop();
      break;
    case Stmt_Return:
      resolveExp(t->ch[1]);
      if(!typeEq(t->ch[1]->exp_type, curFunction->returnType)){
        sdd_error(8, "return type mismatch", t);
      }
      break;
    case Stmt_If:
      resolveExp(t->ch[2]);
      if(!typeEq(t->ch[2]->exp_type, IntType)){
        sdd_error(7, "if condition must be integer", t);
      }
      resolveStmt(t->ch[4]);
      if(t->ch[6]){
        resolveStmt(t->ch[6]);
      }
      break;
    case Stmt_While:
      resolveExp(t->ch[2]);
      if(!(typeEq(t->ch[2]->exp_type, IntType))){
        sdd_error(7, "while condition must be integer", t);
      }
      resolveStmt(t->ch[4]);
      break;
    default:
      assert(0);
  }
}

void resolveDefList_fromCompSt(Tree *t){
  if(t->ch[0]){
    resolveDef_fromCompSt(t->ch[0]);
    resolveDefList_fromCompSt(t->ch[1]);
  }
}

void resolveDef_fromCompSt(Tree *t){
  resolveSpecifier(t->ch[0]);
  t->ch[1]->exp_type = t->ch[0]->exp_type;
  resolveDecList_fromCompSt(t->ch[1]);
}

void resolveDecList_fromCompSt(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  resolveDec_fromCompSt(t->ch[0]);
  if(t->ch[2]){
    t->ch[2]->exp_type = t->exp_type;
    resolveDecList_fromCompSt(t->ch[2]);
  }
}

void resolveDec_fromCompSt(Tree *t){
  assert(t->exp_type);
  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);
  registerVariable(t->ch[0]->var_name, t->ch[0]->exp_type, 0);
}

void resolveDefList_fromStruct(Tree *t){
  t->exp_type = NULL;
  if(t->ch[0]){
    resolveDef_fromStruct(t->ch[0]);
    resolveDefList_fromStruct(t->ch[1]);
    t->exp_type = mergeStruct(t->ch[0]->exp_type, t->ch[1]->exp_type);
  }
}

void resolveDef_fromStruct(Tree *t){
  resolveSpecifier(t->ch[0]);
  t->ch[1]->exp_type = t->ch[0]->exp_type;
  resolveDecList_fromStruct(t->ch[1]);
  t->exp_type = t->ch[1]->exp_type;
}

void resolveDecList_fromStruct(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  resolveDec_fromStruct(t->ch[0]);

  if(t->ch[2]){
    t->ch[2]->exp_type = t->exp_type;
    resolveDecList_fromStruct(t->ch[2]);
    t->exp_type = mergeStruct(t->ch[0]->exp_type, t->ch[2]->exp_type);
  }else{
    t->exp_type = t->ch[0]->exp_type;
  }
}

void resolveDec_fromStruct(Tree *t){
  if(t->ch[2]){
    sdd_error(15, "TODO", t);
  }

  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);
  t->exp_type = makeStruct(t->ch[0]->exp_type, t->ch[0]->var_name);
}

int isValidLeftValue(Tree *t){
  switch(t->int_val){
    case Exp_Id:
    case Exp_QueryArray:
    case Exp_QueryStruct:
      return 1;
    default:
      return 0;
  }
  assert(0);
}

void resolveExp_Op2(Tree *t){
  resolveExp(t->ch[0]);
  resolveExp(t->ch[2]);
  Type *tl = t->ch[0]->exp_type, *tr = t->ch[2]->exp_type;
  switch(t->ch[1]->stype){
    case ASSIGNOP:
      if(!typeEq(tl, tr)){
        sdd_error(5, "assigning unmatching types", t);
      }
      if(!isValidLeftValue(t->ch[0])){
        sdd_error(6, "invalid assignment", t);
      }
      t->exp_type = tl;
      break;
    case AND:
    case OR:
      if(!(typeEq(tl, IntType) && typeEq(tr, IntType))){
        sdd_error(7, "logic OP requires integers", t);
      }
      t->exp_type = IntType;
      break;
    case RELOP:
      if(!typeEq(tl, tr)){
        sdd_error(7, "comparing different types", t);
      }else if(!(typeEq(tl, IntType) || typeEq(tl, FloatType))){
        sdd_error(7, "comparing non-numbers", t);
      }
      t->exp_type = IntType;
      break;
    case PLUS:
    case MINUS:
    case STAR:
    case DIV:
      if(!typeEq(tl, tr)){
        sdd_error(7, "OP on unmatching types", t);
      }else if(!(typeEq(tl, IntType) || typeEq(tl, FloatType))){
        sdd_error(7, "only numbers can do arithmetic OPs", t);
      }
      t->exp_type = tl;
      break;
    default:
      assert(0);
  }
}

void resolveExp_Op1(Tree *t){
  resolveExp(t->ch[1]);
  Type *tl = t->ch[1]->exp_type;
  switch(t->ch[0]->stype){
    case MINUS:
      if(!(typeEq(tl, IntType) || typeEq(tl, FloatType))){
        sdd_error(7, "negation of non-number", t);
      }
      break;
    case NOT:
      if(!(typeEq(tl, IntType))){
        sdd_error(7, "negation of non-integer", t);
      }
      break;
  }
  t->exp_type = t->ch[1]->exp_type;
}

void resolveExp_FunCall(Tree *t){
  const char *name = IDs[t->ch[0]->int_val];
  SymTabEntry *funEntry = trieQuery(symTabFunctions, name);
  if(funEntry){
    List *arg_list;
    if(t->ch[2]){ //has args
      resolveArgs(t->ch[2]);
      arg_list = t->ch[2]->arg_list;
    }else{
      arg_list = newList();
    }
    for(ListNode *param = funEntry->paramList->head, *arg = arg_list->head;param || arg;param = param->next, arg = arg->next){
      if(param == NULL){
        sdd_error(9, "too many arguments", t);
        break;
      }else if(arg == NULL){
        sdd_error(9, "too few arguments", t);
        break;
      }else{
        Param *p = param->val;
        Arg *a = arg->val;
        if(!typeEq(p->type, a->type)){
          sdd_error(9, "argument type mismatch", t);
        }
      }
    }
  }else{
    if(trieQuery(symTabStack.rear->val, name)){
      sdd_error(11, "function call on variable", t);
    }else{
      sdd_error(2, "undefined function", t);
    }
    t->exp_type = IntType;
  }
}

void resolveExp(Tree *t){
  SymTabEntry *entry;
  switch(t->int_val){
    case Exp_Op2:
      resolveExp_Op2(t);
      break;
    case Exp_Parentheses:
      resolveExp(t->ch[1]);
      t->exp_type = t->ch[1]->exp_type;
      break;
    case Exp_Op1:
      resolveExp_Op1(t);
      break;
    case Exp_FunCall:
      resolveExp_FunCall(t);
      break;
    case Exp_QueryArray:
      resolveExp(t->ch[0]);
      resolveExp(t->ch[2]);
      if(t->ch[0]->exp_type->type != 2){
        sdd_error(10, "not an array", t);
        t->exp_type = IntType;
      }else{
        t->exp_type = t->ch[0]->exp_type->next;
      }
      if(!typeEq(t->ch[2]->exp_type, IntType)){
        sdd_error(12, "index should be integer", t);
      }
      break;
    case Exp_QueryStruct:
      resolveExp(t->ch[0]);
      if(t->ch[0]->exp_type->type != 3){
        sdd_error(13, "not a struct", t);
        t->exp_type = IntType;
      }else{
        SymTabEntry *entry = trieQuery(t->ch[0]->exp_type->map, IDs[t->ch[2]->int_val]);
        if(!entry){
          sdd_error(14, "member not found", t);
          t->exp_type = IntType;
        }else{
          t->exp_type = entry->type->structType;
        }
      }
      break;
    case Exp_Id:
      //printf("querying id %s\n", IDs[t->ch[0]->int_val]);
      entry = trieQuery((Trie *)symTabStack.rear->val, IDs[t->ch[0]->int_val]);
      if(entry == NULL){
        sdd_error(1, "undefined variable", t);
        t->exp_type = IntType;
      }else{
        t->exp_type = entry->type;
      }
      break;
    case Exp_Constant:
      if(t->ch[0]->stype == INT){
        t->exp_type = IntType;
      }else if(t->ch[0]->stype == FLOAT){
        t->exp_type = FloatType;
      }
      break;
    default:
      assert(0);
  }
}

void resolveArgs(Tree *t){
  resolveExp(t->ch[0]);
  Arg *arg = malloc(sizeof(Arg));
  arg->type = t->ch[0]->exp_type;
  List *l = newList();
  listAppend(l, arg);

  if(t->ch[2]){
    resolveArgs(t->ch[2]);
    t->arg_list = listMerge(l, t->ch[2]->arg_list);
  }else{
    t->arg_list = l;
  }
}
