typedef struct __Tree{
  struct __Tree *ch;
  union{
    int int_val;
    float float_val;
  };
}Tree;
