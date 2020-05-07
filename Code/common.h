#include <stddef.h>
#define MAXCH 10

enum{
  REL_G,
  REL_L,
  REL_GE,
  REL_LE,
  REL_EQ,
  REL_NE,
};

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
  Exp_ASSIGN,
  Exp_AND,
  Exp_OR,
  Exp_RELOP,
  Exp_PLUS,
  Exp_MINUS,
  Exp_STAR,
  Exp_DIV,
  Exp_Parentheses,
  Exp_NEG,
  Exp_NOT,
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

/********* IntListHash ******/
#define HASH_NUM 3
#define LIST_HASH_MAX 5

static const int MOD[HASH_NUM] = { // a list of different large primes
  1372212617, 1000000007, 998244353,
};

typedef struct __IntListHash{
  int v[HASH_NUM], len[HASH_NUM];
}IntListHash;

/*########### Type ########*/
struct __Trie;
typedef struct __Type{
  int type; // 0 for int, 1 for float, 2 for array, 3 for struct
  IntListHash hash; //used for determine struct eq

  int totsize;

  union{
    struct{
      int size; //for array
      struct __Type *next;
    };
    struct{
      List *structList;
      struct __Trie *map;
    };
  };

}Type;

/*########### Param ########*/
typedef struct{
  Type *type;
  const char *name;
}Param;

typedef struct{
  Type *type;
}Arg;

typedef struct{
  Type *type;
  const char *name;
  int offset;
}StructEntry;


/*###########symbol table########*/
typedef struct __SymTableEmtry{
  const char *name;
  int depth;
  union{
    struct{ // for variables and structEntrys
      Type *type;
      StructEntry *structEntry;
      int label;
    };
    struct{ // for functions
      Type *returnType;
      List *paramList;
      int defined, lineno;
    };
  };
}SymTabEntry;

/*############ Trie #############*/
typedef struct __Trie{
  struct __Trie *go[26+26+10+1];
  SymTabEntry *entry;
  int depth;
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

  Type *exp_type, *struct_type;
  const char *var_name;
  List *var_list, *arg_list;

  int label;
}Tree;

typedef Tree *Treep;

/********* IR ************/
typedef struct{
  int op;
  union{
    int dst;
    const char *dst_var;
  };
  union{
    int src1;
    const char *src1_var;
  };
  int cnst1;
  union{
    int src2;
    const char *src2_var;
  };
  int cnst2;
}IRCode;

enum{
  OP_LABEL, //LABEL Label#src1

  OP_FUNCTION, //FUNCTION src1_var
  OP_PARAM, //PARAM t#src1
  OP_FUNCALL, // t#dst := CALL src1_val
  OP_ARG, //ARG t#src1
  OP_DEC, //DEC t#src1 src2

  OP_READ, //READ t#dst
  OP_WRITE, //WRITE t#src1

  OP_ASSIGN, //t#dst := t#src1
  OP_LOAD_IMM, //t#dst := constant(src1)
  OP_GETADDR, //t#dst := &t#src1
  OP_PUTADDR, //*t#dst := t#src1
  OP_GETFROMADDR, //t#dst := *t#src1

  OP_GOTO, //GOTO Label#dst
  OP_IFG_GOTO, //IF t#src1 > t#src2 GOTO Label#dst
  OP_IFL_GOTO,
  OP_IFGE_GOTO,
  OP_IFLE_GOTO,
  OP_IFEQ_GOTO,
  OP_IFNE_GOTO,

  OP_ADD, //t#dst := t#src1 + t#src2
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_RETURN, //RETURN t#src1
};

/********* vector *********/
typedef struct{
  IRCode *data;
  size_t size, len;
}Vector;
