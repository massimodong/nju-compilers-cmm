#include "common.h"
#include <stdlib.h>

extern int label_cnt;
Vector *vector_new();
void vector_free(Vector *);
void vec_pb(Vector *, IRCode);

int has_dst(IRCode irc);

static int *bno, *active, *rem, *label2block, *block_start, *block_end, **block_list, *block_list_len, **block_in, *block_in_len;
static int block_cnt = 0;

static void generate_blocks(Vector *vec){
  for(int i=0;i<vec->len;++i){
    IRCode irc = vec->data[i];
    switch(irc.op){
      case OP_FUNCTION: //start of new block
      case OP_LABEL:
        bno[i] = ++block_cnt;
        if(irc.op == OP_LABEL){
          label2block[irc.src1] = block_cnt;
        }
        break;
      case OP_RETURN: //end of pre block
      case OP_GOTO:
      case OP_IFG_GOTO:
      case OP_IFL_GOTO:
      case OP_IFGE_GOTO:
      case OP_IFLE_GOTO:
      case OP_IFEQ_GOTO:
      case OP_IFNE_GOTO:
        bno[i] = block_cnt;
        if(i+1<vec->len && vec->data[i+1].op != OP_FUNCTION && vec->data[i+1].op != OP_LABEL) ++block_cnt;
        break;
      default:
        bno[i] = block_cnt;
        break;
    }
  }

  block_start = malloc(sizeof(int) * (block_cnt + 233));
  block_end = malloc(sizeof(int) * (block_cnt + 233));

  //for(int i=0;i<vec->len;++i) printf("%d %d\n", bno[i], vec->data[i].op);
  block_start[1] = 0;
  for(int i=1;i<vec->len;++i){
    if(bno[i] != bno[i-1]) block_start[bno[i]] = i;
    block_end[bno[i]] = i;
  }

  //block list
  block_list_len = malloc(sizeof(int) * (block_cnt + 233));
  block_list = malloc(sizeof(int *) * (block_cnt + 233));
  block_in_len = malloc(sizeof(int) * (block_cnt + 233));
  block_in = malloc(sizeof(int *) * (block_cnt + 233));
  for(int i=1;i<=block_cnt;++i){
    block_list_len[i] = 0;
    block_list[i] = malloc(sizeof(int) * (vec->len + 233));
    block_in_len[i] = 0;
    block_in[i] = malloc(sizeof(int) * (block_cnt + 233));
  }

  for(int b=1;b<=block_cnt;++b){
    IRCode irc = vec->data[block_end[b]];
    int t;
    switch(irc.op){
      case OP_GOTO:
        t = label2block[irc.dst];
        block_in[t][block_in_len[t]++] = b;
        break;
      case OP_IFG_GOTO:
      case OP_IFL_GOTO:
      case OP_IFGE_GOTO:
      case OP_IFLE_GOTO:
      case OP_IFEQ_GOTO:
      case OP_IFNE_GOTO:
        t = label2block[irc.dst];
        block_in[t][block_in_len[t]++] = b;
        if(b+1 <= block_cnt && vec->data[block_start[b+1]].op != OP_FUNCTION){
          t = b+1;
          block_in[t][block_in_len[t]++] = b;
        }
        break;
      case OP_RETURN:
        break;
      default:
        if(b+1 <= block_cnt && vec->data[block_start[b+1]].op != OP_FUNCTION){
          t = b+1;
          block_in[t][block_in_len[t]++] = b;
        }
        break;
    }
  }
}

static int has_src1(IRCode irc){
  switch(irc.op){
    case OP_ARG:
    case OP_ASSIGN:
    case OP_GETADDR:
    case OP_PUTADDR:
    case OP_GETFROMADDR:
    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      return irc.cnst1 == 0;
    default:
      return 0;
  }
}

static int has_src2(IRCode irc){
  switch(irc.op){
    case OP_PUTADDR:
    case OP_IFG_GOTO:
    case OP_IFL_GOTO:
    case OP_IFGE_GOTO:
    case OP_IFLE_GOTO:
    case OP_IFEQ_GOTO:
    case OP_IFNE_GOTO:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
      return irc.cnst2 == 0;
    default:
      return 0;
  }
}

static int resolve_easy(IRCode irc){
  switch(irc.op){
    case OP_LABEL:
    case OP_FUNCTION:
    case OP_PARAM:
    case OP_DEC:
    case OP_GOTO:
      return 1;

    case OP_WRITE:
    case OP_RETURN:
    case OP_ARG:
      if(irc.cnst1 == 0) active[irc.src1] = 1;
      return 1;

    default:
      return 0;
  }
}

static void opt_work(Vector *vec){
  for(int b = block_cnt;b;--b){
    for(int i=0;i<block_list_len[b];++i) active[block_list[b][i]] = 1;

    switch(vec->data[block_end[b]].op){ // if jump backwards
      case OP_GOTO:
      case OP_IFG_GOTO:
      case OP_IFL_GOTO:
      case OP_IFGE_GOTO:
      case OP_IFLE_GOTO:
      case OP_IFEQ_GOTO:
      case OP_IFNE_GOTO:
        {
          int t = label2block[vec->data[block_end[b]].dst];
          if(t <= b){
            for(int j=block_start[t];j<=block_end[b];++j){
              IRCode irc = vec->data[j];
              if(!resolve_easy(irc)){
                //ignore READ
                if(has_src1(irc)) active[irc.src1] = 1;
                if(has_src2(irc)) active[irc.src2] = 1;
              }
            }
          }
        }
    }

    for(int i=block_end[b];i>=block_start[b];--i){
      IRCode irc = vec->data[i];
      if(!resolve_easy(irc)){
        if(irc.op == OP_READ){
          active[irc.dst] = 0;
        }else if(has_dst(irc) && !active[irc.dst]){
          rem[i] = 0;
        }else{
          if(has_dst(irc)) active[irc.dst] = 0;
          if(has_src1(irc)) active[irc.src1] = 1;
          if(has_src2(irc)) active[irc.src2] = 1;
        }
      }
    }

    int *go = block_in[b], go_cnt = block_in_len[b];

    for(int i=1;i<=label_cnt;++i) if(active[i]){
      for(int j=0;j<go_cnt;++j){
        block_list[go[j]][block_list_len[go[j]]++] = i;
      }
      active[i] = 0;
    }
  }
}

void opt_block(Vector *vec){
  Vector *nv = vector_new();
  block_cnt = 0;

  bno = malloc(sizeof(int) * (vec->len + 233));
  active = malloc(sizeof(int) * (label_cnt + 233));
  rem = malloc(sizeof(int) * (vec->len + 233));
  label2block = malloc(sizeof(int) * (label_cnt + 233));
  for(int i=1;i<=label_cnt;++i){
    active[i] = 0;
    label2block[i] = 0;
  }

  for(int i=0;i<vec->len;++i) rem[i] = 1;

  generate_blocks(vec);
  //printf("block_cnt: %d\n", block_cnt);
  opt_work(vec);

  for(int i=0;i<vec->len;++i) if(rem[i]) vec_pb(nv, vec->data[i]);
  vec->len = nv->len;
  for(int i=0;i<nv->len;++i) vec->data[i] = nv->data[i];

  for(int i=1;i<=block_cnt;++i){
    free(block_list[i]);
    free(block_in[i]);
  }
  free(block_list);
  free(block_list_len);
  free(block_in);
  free(block_in_len);

  free(bno);
  free(block_start);
  free(block_end);
  free(active);
  free(rem);
  free(label2block);
  vector_free(nv);
}
