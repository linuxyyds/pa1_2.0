/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>



//extern const char* regsl[];
//extern const char* regsw[];
//extern const char* regsb[];



word_t expr(char *e, bool *success);

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
 
  /* TODO: Add more members if necessary */
  char EXPR[32];
  word_t VALUE;
  uint32_t val;
  char exprs[32];
 // char *exprs;
} WP;

void free_wp(WP *wp) ;
void print_w();
WP* delete_wp(int id, bool* f) ;
void check_wp(bool* f) ;
WP* new_wp() ;
#endif
