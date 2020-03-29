#define MAXCH 10

enum{
  ExtDef_Val,
  ExtDef_Func,
  Specifier_Type,
  Specifier_Struct,
  VarDec_Id,
  VarDec_Array,
};

#define Syntaxes(ACTION)\
  ACTION(Program)\
  ACTION(ExtDefList)\
  ACTION(ExtDef)\
  ACTION(ExtDecList)\
  ACTION(Specifier)\
  ACTION(StructSpecifier)\
  ACTION(OptTag)\
  ACTION(Tag)\
  ACTION(VarDec)\
  ACTION(FunDec)\
  ACTION(VarList)\
  ACTION(ParamDec)\
  ACTION(CompSt)\
  ACTION(StmtList)\
  ACTION(Stmt)\
  ACTION(DefList)\
  ACTION(Def)\
  ACTION(DecList)\
  ACTION(Dec)\
  ACTION(Exp)\
  ACTION(Args)\


#define GENERATE_ENUM(e) e,
#define GENERATE_STRING(s) #s,

enum{
  Syntaxes(GENERATE_ENUM)
};

static const char *syntaxName[] = {
  Syntaxes(GENERATE_STRING)
};

/*########### List ########*/
typedef struct __ListNode{
  struct __ListNode *last, *next;
  void *val;
}ListNode;

typedef struct{
  ListNode *head, *rear;
}List;

/*########### Type ########*/
typedef struct __Type{
  int type; // 0 for int, 1 for float, 2 for array, 3 for struct

  union{
    int size; //for array
    struct{
      const char *name; //for struct
      struct __Type *structType;
    };
  };

  struct __Type *next, *last;
}Type;

/*########### Param ########*/
typedef struct{
  Type *type;
  const char *name;
}Param;

/*###########symbol table########*/
typedef struct __SymTableEmtry{
  const char *name;
  union{
    struct{
      int isStructDec;
      Type *type;
    };
    struct{
      Type *returnType;
      List *paramList;
      int defined;
    };
  };
}SymTabEntry;

/*############ Trie #############*/
typedef struct __Trie{
  struct __Trie *go[26+26+10+1];
  int depth;
  SymTabEntry *entry;
}Trie;

/************ Tree ***************/
typedef struct __Tree{
  int stype, show;
  struct __Tree *ch[MAXCH];
  int lineno;
  union{
    unsigned int int_val;
    float float_val;
  };

  Type *exp_type;
  const char *var_name;
}Tree;

typedef Tree *Treep;
