#define MAXCH 10

enum{
  ExtDef_Val,
  ExtDef_Func,
  Specifier_Type,
  Specifier_Struct,
  VarDec_Id,
  VarDec_Array,
  Stmt_Exp,
  Stmt_CompSt,
  Stmt_Return,
  Stmt_If,
  Stmt_While,
  Exp_Op2,
  Exp_Parentheses,
  Exp_Op1,
  Exp_FunCall,
  Exp_QueryArray,
  Exp_QueryStruct,
  Exp_Id,
  Exp_Constant,
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
  struct __ListNode *prev, *next;
  void *val;
}ListNode;

typedef struct{
  ListNode *head, *rear;
}List;

/*########### Type ########*/
struct __Trie;
typedef struct __Type{
  int type; // 0 for int, 1 for float, 2 for array, 3 for struct

  union{
    int size; //for array
    struct{
      const char *name; //for struct
      struct __Type *structType;
      struct __Trie *map;
    };
  };

  struct __Type *next, *last;
}Type;

/*########### Param ########*/
typedef struct{
  Type *type;
  const char *name;
}Param;

typedef struct{
  Type *type;
}Arg;


/*###########symbol table########*/
typedef struct __SymTableEmtry{
  const char *name;
  int depth;
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
  List *var_list, *arg_list;
}Tree;

typedef Tree *Treep;
