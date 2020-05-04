#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "syntax.tab.h"

//#define PRINTREGISTERS 1

void sdd_error_lineno(int n, const char *msg, int lineno){
  printf("Error type %d at Line %d: %s.\n", n, lineno, msg);
}

extern char **IDs;
int trieInsert(Trie **, const char *, SymTabEntry *);
SymTabEntry *trieQuery(Trie *, const char *);
void listAppend(List *, void *);
void listPopRear(List *);
List *listMerge(List *, List *);
List *newList();
IntListHash ListHashAppendNum(IntListHash, int);
IntListHash ListHashAppend(IntListHash, IntListHash);
void ListHashInit(IntListHash *, int);
int ListHashEq(IntListHash a, IntListHash b);

void irInit();
void irFinish();
void irFunc(Tree *);

Type *IntType, *FloatType;

Type *makeArray(Type *t, int n){
  Type *ret = malloc(sizeof(Type));
  ret->type = 2;
  ret->totsize = n * t->totsize;
  ret->size = n;
  ret->next = t;
  ListHashInit(&ret->hash, 2);
  ret->hash = ListHashAppend(ret->hash, t->hash);
  return ret;
}
StructEntry *makeStructEntry(Type *t, const char *name){
  assert(t);
  StructEntry *se = malloc(sizeof(StructEntry));
  se->type = t;
  se->name = name;
  return se;
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
  /*
  if(t1->type == 0) return t2->type == 0;
  else if(t1->type == 1) return t2->type == 1;
  else if(t1->type == 2){
    if(t2->type != 2) return 0;
    return typeEq(t1->next, t2->next);
  }else if(t1->type == 3){
    if(t2->type != 3) return 0;
    for(ListNode *n1 = t1->structList->head, *n2 = t2->structList->head;n1||n2;n1 = n1->next, n2 = n2->next){
      if(n1 == NULL || n2 == NULL){
        return 0;
      }
      StructEntry *se1 = n1->val, *se2 = n2->val;
      if(!typeEq(se1->type, se2->type)) return 0;
    }
    return 1;
  }else{
    assert(0);
  }
  */
  return ListHashEq(t1->hash, t2->hash);
}

void printType(Type *t){
  assert(t);
  if(t->type == 0) printf("int");
  else if(t->type == 1) printf("float");
  else if(t->type == 2){
    printf("array(%d, ", t->size);
    printType(t->next);
    printf(")");
  }else{
    assert(t->type == 3);
    printf("struct{");
    for(ListNode *n = t->structList->head; n; n = n->next){
      StructEntry *se = n->val;
      printf("(%s: ", se->name);
      assert(se->type);
      printType(se->type);
      printf(")");
    }
    printf("}");
  }
  printf(" (%d) -- (size: %d)", t->hash.v[0], t->totsize);
}

Trie *symTabStructs;
List symTabStack = (List){NULL, NULL};
int symTabStackDepth = 0;

// should not implement this in lab3
void symTabStackPush(){
  /*
  listAppend(&symTabStack, symTabStack.rear->val);
  ++symTabStackDepth;
  */
}
void symTabStackPop(){
  /*
  --symTabStackDepth;
  listPopRear(&symTabStack);
  */
}

/* register a variable, return whether success
 */
extern int label_cnt;
void registerVariable(const char *name, Type *type, int lineno){
#ifdef PRINTREGISTERS
  printf("register variable %s of type: ", name);
  printType(type);
  printf(" height: %d\n", symTabStackDepth);
#endif

  SymTabEntry *structEntry = trieQuery(symTabStructs, name);
  if(structEntry){
    sdd_error_lineno(3, "struct name can't be used as variable name", lineno);
    return;
  }

  SymTabEntry *varEntry = trieQuery(symTabStack.rear->val, name);
  if(varEntry && varEntry->depth == symTabStackDepth){
    sdd_error_lineno(3, "variable redefined", lineno);
    return;
  }

  SymTabEntry *entry= malloc(sizeof(SymTabEntry));
  entry->name = name;
  entry->depth = symTabStackDepth;
  entry->type = type;
  entry->label = ++label_cnt;

  trieInsert(&symTabStack.rear->val, name, entry);
}

void registerStruct(const char *name, Type *type, int lineno){
#ifdef PRINTREGISTERS
  printf("register struct %s{", name);
  for(ListNode *n = type->structList->head;n;n=n->next){
    StructEntry *entry = n->val;
    printf("%s ", entry->name);
    printType(entry->type);
    if(n->next) printf(", ");
  }
  printf("}\n");
#endif

  SymTabEntry *varEntry = trieQuery(symTabStack.rear->val, name);
  if(varEntry){
    sdd_error_lineno(16, "variable name can't be used as struct name", lineno);
    return;
  }

  SymTabEntry *structEntry = trieQuery(symTabStructs, name);
  if(structEntry){
    sdd_error_lineno(16, "struct redefined", lineno);
    return;
  }

  SymTabEntry *entry = malloc(sizeof(SymTabEntry));
  entry->name = name;
  entry->depth = 0;
  entry->type = type;

  trieInsert(&symTabStructs, name, entry);
}

Trie *symTabFunctions = NULL;
SymTabEntry *curFunction;
List functionsList = (List){NULL, NULL};
int registerFunction(const char *name, Type *retType, List *paramList, int lineno){
#ifdef PRINTREGISTERS
  printf("register function ");
  printType(retType);
  printf(" <%s>(", name);
  for(ListNode *n = paramList->head; n; n=n->next){
    Param *p = n->val;
    printType(p->type);
    printf(" %s,", p->name);
  }
  printf(")\n");
#endif

  SymTabEntry *entry = malloc(sizeof(SymTabEntry));
  entry->name = name;
  entry->depth = 0;
  entry->returnType = retType;
  entry->paramList = paramList;
  entry->defined = 0;
  entry->lineno = lineno;

  if(trieInsert(&symTabFunctions, name, entry)){
    listAppend(&functionsList, entry);
    return 1;
  }else{
    return 0;
  }
}

void sdd_error(int n, const char *msg, Tree *t){
  sdd_error_lineno(n, msg, t->lineno);
}

void checkUndefinedFunctions(){
  for(ListNode *n = functionsList.head;n;n=n->next){
    SymTabEntry *entry = n->val;
    if(!entry->defined){
      sdd_error_lineno(18, "declared function has no definition", entry->lineno);
    }
  }
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
void resolveDec_fromCompSt(Tree *); //inh: type; registers variables-e

void resolveDefList_fromStruct(Tree *); //inh: struct_type (to be filled)
void resolveDef_fromStruct(Tree *); //inh: struct_type (to be filled)
void resolveDecList_fromStruct(Tree *); //inh: struct_type (to be filled), exp_type
void resolveDec_fromStruct(Tree *); //inh: struct_type (to be filled), exp_type

void resolveExp(Tree *); //syn: exp_type
void resolveArgs(Tree *); //syn: arg_list

void resolveProgram(Tree *t){
  IntType = malloc(sizeof(Type));
  FloatType = malloc(sizeof(Type));
  IntType->type = 0;
  FloatType->type = 1;
  ListHashInit(&IntType->hash, 0);
  ListHashInit(&FloatType->hash, 1);
  IntType->totsize = FloatType->totsize = 4;

  listAppend(&symTabStack, NULL);
  symTabStackDepth = 1;

  registerFunction("read", IntType, newList(), 0);
  registerFunction("write", IntType, makeParam("13107FF", IntType), 0);
  trieQuery(symTabFunctions, "read")->defined = 1;
  trieQuery(symTabFunctions, "write")->defined = 1;

  irInit();
  resolveExtDefList(t->ch[0]);

  checkUndefinedFunctions();
  irFinish();
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
        for(ListNode *n=e->paramList->head;n;n=n->next){
          Param *p = n->val;
          registerVariable(p->name, p->type, t->lineno);
        }
        resolveCompSt(t->ch[2]);
        irFunc(t);
      }
      symTabStackPop();
      break;
  }
}

void resolveExtDecList(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);
  Type *type = t->ch[0]->exp_type;
  const char *name = t->ch[0]->var_name;
  registerVariable(name, type, t->ch[0]->lineno);
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
      type = malloc(sizeof(Type));
      type->type = 3;
      type->structList = newList();
      type->map = NULL;

      t->ch[3]->struct_type = type;
      symTabStackPush();
      resolveDefList_fromStruct(t->ch[3]);
      symTabStackPop();

      int totsize = 0;

      ListHashInit(&type->hash, 3);
      for(ListNode *n = type->structList->head;n;n=n->next){
        StructEntry *entry = n->val;
        type->hash = ListHashAppend(type->hash, entry->type->hash);

        entry->offset = totsize;
        totsize += entry->type->totsize;
      }
      type->hash = ListHashAppendNum(type->hash, 4);

      type->totsize = totsize;
    }
    if(t->ch[1]->show){ //Tag name is not empty
      const char *name = IDs[t->ch[1]->ch[0]->int_val];
      if(type){ //if struct has body
        registerStruct(name, type, t->lineno);
      }else{
        SymTabEntry *e = trieQuery(symTabStructs, name);
        if(e){
          type = e->type;
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
    registerFunction(t->var_name, t->exp_type, paramList, t->lineno);
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
  registerVariable(t->ch[0]->var_name, t->ch[0]->exp_type, t->ch[0]->lineno);

  if(t->ch[2]){
    resolveExp(t->ch[2]);
    if(!typeEq(t->ch[0]->exp_type, t->ch[2]->exp_type)){
      sdd_error(5, "variable initialized with inconsistent type", t);
    }
  }
}

void resolveDefList_fromStruct(Tree *t){
  if(t->ch[0]){
    t->ch[0]->struct_type = t->struct_type;
    t->ch[1]->struct_type = t->struct_type;
    resolveDef_fromStruct(t->ch[0]);
    resolveDefList_fromStruct(t->ch[1]);
  }
}

void resolveDef_fromStruct(Tree *t){
  resolveSpecifier(t->ch[0]);
  t->ch[1]->exp_type = t->ch[0]->exp_type;
  t->ch[1]->struct_type = t->struct_type;
  resolveDecList_fromStruct(t->ch[1]);
}

void resolveDecList_fromStruct(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  t->ch[0]->struct_type = t->struct_type;
  resolveDec_fromStruct(t->ch[0]);

  if(t->ch[2]){
    t->ch[2]->exp_type = t->exp_type;
    t->ch[2]->struct_type = t->struct_type;
    resolveDecList_fromStruct(t->ch[2]);
  }
}

void resolveDec_fromStruct(Tree *t){
  if(t->ch[2]){
    sdd_error(15, "you can't initialize a struct member", t);
  }

  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);

  StructEntry *se = makeStructEntry(t->ch[0]->exp_type, t->ch[0]->var_name);
  assert(se);
  assert(t->struct_type);
  assert(t->struct_type->structList);
  listAppend(t->struct_type->structList, se);

  if(trieQuery(t->struct_type->map, se->name)){
    sdd_error(15, "redefined struct member", t);
  }else{
    SymTabEntry *entry = malloc(sizeof(SymTabEntry));
    entry->name = se->name;
    entry->depth = 0;
    entry->type = se->type;
    entry->structEntry = se;
    trieInsert(&t->struct_type->map, se->name, entry);
  }
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
    t->exp_type = funEntry->returnType;
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
    case Exp_ASSIGN:
    case Exp_AND:
    case Exp_OR:
    case Exp_RELOP:
    case Exp_PLUS:
    case Exp_MINUS:
    case Exp_STAR:
    case Exp_DIV:
      resolveExp_Op2(t);
      break;
    case Exp_Parentheses:
      resolveExp(t->ch[1]);
      t->exp_type = t->ch[1]->exp_type;
      break;
    case Exp_NEG:
    case Exp_NOT:
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
          t->exp_type = entry->type;
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
