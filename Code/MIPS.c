#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

extern FILE *fir;
extern int label_cnt;

static const char *START_TEXT[] = {
  ".data",
  "_prompt: .asciiz \"Please input an integer:\\n\"",
  "_ret: .asciiz \"\\n\"",
  ".globl main",
  ".text",
};
#define START_TEXT_LEN (sizeof(START_TEXT)/sizeof(char *))

/*
 * status: 0 undefined; 1 local variable; 2 parameter variable
 */
int *var_pos, *var_status, pos_cnt, param_cnt;

static void init(){
  var_pos = malloc(sizeof(int) * (label_cnt + 233));
  var_status = malloc(sizeof(int) * (label_cnt + 233));

  for(int i=0;i<START_TEXT_LEN;++i) fprintf(fir, "%s\n", START_TEXT[i]);
}

static void register_variable_dst(IRCode irc){
  if(!var_status[irc.dst]){
    var_pos[irc.dst] = -(++pos_cnt);
    var_status[irc.dst] = 1;
  }
}

static void register_variable_src1(IRCode irc){
  if(!irc.cnst1 && !var_status[irc.src1]){
    var_pos[irc.src1] = -(++pos_cnt);
    var_status[irc.src1] = 1;
  }
}

static void register_variable_src2(IRCode irc){
  if(!irc.cnst2 && !var_status[irc.src2]){
    var_pos[irc.src2] = -(++pos_cnt);
    var_status[irc.src2] = 1;
  }
}

static void find_variables(Vector *vec, int s){
  for(int i=s+1;i<vec->len && vec->data[i].op != OP_FUNCTION;++i){
    IRCode irc = vec->data[i];
    switch(irc.op){
      case OP_LABEL:
      case OP_FUNCTION:
      case OP_GOTO:
        break;

      case OP_PARAM:
        var_pos[irc.dst] = param_cnt++;
        var_status[irc.dst] = 1;
        break;

      case OP_DEC:
        pos_cnt += irc.src2/4;
        var_pos[irc.src1] = -pos_cnt;
        var_status[irc.src1] = 1;
        break;

      //dst:
      case OP_FUNCALL:
      case OP_READ:
        register_variable_dst(irc);
        break;

      //src1
      case OP_ARG:
      case OP_WRITE:
      case OP_RETURN:
        register_variable_src1(irc);
        break;

      //dst, src1
      case OP_ASSIGN:
      case OP_GETADDR:
      case OP_GETFROMADDR:
        register_variable_dst(irc);
        register_variable_src1(irc);
        break;

      //src1, src2
      case OP_PUTADDR:
      case OP_IFG_GOTO:
      case OP_IFL_GOTO:
      case OP_IFGE_GOTO:
      case OP_IFLE_GOTO:
      case OP_IFEQ_GOTO:
      case OP_IFNE_GOTO:
        register_variable_src1(irc);
        register_variable_src2(irc);
        break;

      //dst, src1, src2
      case OP_ADD:
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
        register_variable_dst(irc);
        register_variable_src1(irc);
        register_variable_src2(irc);
        break;
      default:
        assert(0);
    }
  }

  for(int i=0;i<=label_cnt;++i) if(var_status[i]) var_pos[i] *= 4;
}

static void loads1(IRCode irc){
  if(irc.cnst1){
    fprintf(fir, "  li $t1, %d\n", irc.src1);
  }else{
    fprintf(fir, "  lw $t1, %d($fp)\n", var_pos[irc.src1]);
  }
}

static void loads2(IRCode irc){
  if(irc.cnst2){
    fprintf(fir, "  li $t2, %d\n", irc.src2);
  }else{
    fprintf(fir, "  lw $t2, %d($fp)\n", var_pos[irc.src2]);
  }
}

static void savedst(IRCode irc, const char *str){
  fprintf(fir, "  sw %s, %d($fp)\n", str, var_pos[irc.dst]);
}

static void translateStmt(IRCode irc){
  switch(irc.op){
    case OP_LABEL:
      fprintf(fir, "label%d:\n", irc.src1);
      break;

    case OP_FUNCTION:
    case OP_PARAM:
    case OP_DEC:
      break;

    case OP_FUNCALL:
      if(strcmp(irc.src1_var, "main") == 0) fprintf(fir, "  jal main\n");
      else fprintf(fir, "  jal fun_%s\n", irc.src1_var);
      savedst(irc, "$v0");
      //fprintf(fir, "  sw $v0, %d($fp)\n", var_pos[irc.dst]);
      break;

    case OP_ARG:
      fprintf(fir, "  addi $sp, $sp, -4\n");
      loads1(irc);
      fprintf(fir, "  sw $t1, 0($sp)\n");
      break;

    case OP_READ:
      //fprintf(fir, "  la $a0, _prompt\n");
      //fprintf(fir, "  li $v0, 4\n");
      //fprintf(fir, "  syscall\n");

      fprintf(fir, "  li $v0, 5\n");
      fprintf(fir, "  syscall\n");
      savedst(irc, "$v0");
      break;

    case OP_WRITE:
      loads1(irc);
      fprintf(fir, "  move $a0, $t1\n");
      fprintf(fir, "  li $v0, 1\n");
      fprintf(fir, "  syscall\n");
      fprintf(fir, "  la $a0, _ret\n");
      fprintf(fir, "  li $v0, 4\n");
      fprintf(fir, "  syscall\n");
      break;

    case OP_ASSIGN:
      loads1(irc);
      savedst(irc, "$t1");
      break;

    case OP_GETADDR:
      fprintf(fir, "  addi $t0, $fp, %d\n", var_pos[irc.src1]);
      savedst(irc, "$t0");
      break;

    case OP_PUTADDR:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sw $t1, 0($t2)\n");
      break;

    case OP_GETFROMADDR:
      loads1(irc);
      fprintf(fir, "  lw $t0, 0($t1)\n");
      savedst(irc, "$t0");
      break;

    case OP_GOTO:
      fprintf(fir, "  j label%d\n", irc.dst);
      break;

    case OP_IFG_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sub $t0, $t1, $t2\n");
      fprintf(fir, "  bgtz $t0, label%d\n", irc.dst);
      break;

    case OP_IFL_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sub $t0, $t1, $t2\n");
      fprintf(fir, "  bltz $t0, label%d\n", irc.dst);
      break;

    case OP_IFGE_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sub $t0, $t1, $t2\n");
      fprintf(fir, "  bgez $t0, label%d\n", irc.dst);
      break;

    case OP_IFLE_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sub $t0, $t1, $t2\n");
      fprintf(fir, "  blez $t0, label%d\n", irc.dst);
      break;

    case OP_IFEQ_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  beq $t1, $t2, label%d\n", irc.dst);
      break;

    case OP_IFNE_GOTO:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  bne $t1, $t2, label%d\n", irc.dst);
      break;

    case OP_ADD:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  add $t0, $t1, $t2\n");
      savedst(irc, "$t0");
      break;

    case OP_SUB:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  sub $t0, $t1, $t2\n");
      savedst(irc, "$t0");
      break;

    case OP_MUL:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  mult $t1, $t2\n");
      fprintf(fir, "  mflo $t0\n");
      savedst(irc, "$t0");
      break;

    case OP_DIV:
      loads1(irc);
      loads2(irc);
      fprintf(fir, "  div $t1, $t2\n");
      fprintf(fir, "  mflo $t0\n");
      savedst(irc, "$t0");
      break;

    case OP_RETURN:
      loads1(irc);
      fprintf(fir, "  move $v0, $t1\n");
      fprintf(fir, "  move $sp, $fp\n");
      fprintf(fir, "  addi $sp, $sp, %d\n", param_cnt * 4);
      fprintf(fir, "  lw $ra, 0($fp)\n");
      fprintf(fir, "  lw $fp, 4($fp)\n");
      fprintf(fir, "  jr $ra\n");
      break;

    default:
      assert(0);
  }
}

static void printFunction(Vector *vec, int s){
  for(int i=0;i<=label_cnt;++i) var_pos[i] = var_status[i] = 0;
  pos_cnt = 0;
  param_cnt = 2;
  find_variables(vec, s);

  if(strcmp(vec->data[s].src1_var, "main") == 0) fprintf(fir, "main:\n");
  else fprintf(fir, "fun_%s:\n", vec->data[s].src1_var);
  fprintf(fir, "  addi $sp, $sp, -8\n");
  fprintf(fir, "  sw $ra, 0($sp)\n");
  fprintf(fir, "  sw $fp, 4($sp)\n");
  fprintf(fir, "  move $fp, $sp\n");
  fprintf(fir, "  addi $sp, $sp, -%d\n", pos_cnt * 4);
  for(int i=s+1;i<vec->len && vec->data[i].op != OP_FUNCTION;++i){
    translateStmt(vec->data[i]);
  }

  fprintf(fir, "\n");
}

void printMIPS(Vector *vec){
  init();
  for(int i=0;i<vec->len;++i){
    if(vec->data[i].op == OP_FUNCTION){
      printFunction(vec, i);
    }
  }
}
