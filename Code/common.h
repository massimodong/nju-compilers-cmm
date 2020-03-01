#define MAXCH 10

typedef struct __Tree{
  struct __Tree *ch[MAXCH];
  union{
    int int_val;
    float float_val;
  };
}Tree;

enum{
  Program,
  ExtDef,
  ExtDef_Val,
  ExtDef_Func,
};
