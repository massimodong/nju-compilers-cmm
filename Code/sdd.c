#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern char **IDs;

Type *IntType, *FloatType; //TODO: need initialized

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

/* register a variable, if collision, output error
 */
void registerVariable(const char *name, Type *type, int isStructDec){
  if(isStructDec){
    printf("register struct %s as: ", name);
    printType(type);
    printf("\n");
  }else{
    printf("register variable %s of type: ", name);
    printType(type);
    printf("\n");
  }
  return;
  assert(0);
  //TODO
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
//gap
void resolveDefList_fromStruct(Tree *); //syn: type
void resolveDef_fromStruct(Tree *); //syn: type
void resolveDecList_fromStruct(Tree *); //inh: type; syn: type (struct to be merged)
void resolveDec_fromStruct(Tree *); //inh: type; syn: type (struct of size 1)

void resolveProgram(Tree *t){
  IntType = newType();
  FloatType = newType();
  IntType->type = 0;
  FloatType->type = 1;
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
        if(t->ch[0]->exp_type == NULL){
          sdd_error(17, "TODO", t);
          t->ch[1]->exp_type = IntType;
        }else{
          t->ch[1]->exp_type = t->ch[0]->exp_type;
        }
        resolveExtDecList(t->ch[1]);
      }
      break;
    case ExtDef_Func:
      if(t->ch[0]->exp_type == NULL){
        sdd_error(17, "TODO", t);
      }
      assert(0);
      //TODO: need to modify syntax.y
      break;
  }
}

void resolveExtDecList(Tree *t){
  t->ch[0]->exp_type = t->exp_type;
  resolveVarDec(t->ch[0]);
  t->exp_type = t->ch[0]->exp_type;
  t->var_name = t->ch[0]->var_name;
  registerVariable(t->var_name, t->exp_type, 0);
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
      resolveDefList_fromStruct(t->ch[3]);
      type = t->ch[3]->exp_type;
    }
    if(t->ch[1]->show){ //Tag name is not empty
      registerVariable(IDs[t->ch[1]->ch[0]->int_val], type, 1);
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
