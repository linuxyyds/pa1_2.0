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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

void init_regex();
void init_wp_pool();
#define TestCorrect(x) if(x){printf("Invalid Command!\n");return 0;}

static int is_batch_mode = false;
float mixmodel(char*s1);
 
 

static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
	int step;
	if (args == NULL) step = 1;
	else sscanf(args, "%d", &step);
	cpu_exec(step);
	return 0;
}

static int cmd_p(char *args) {
  char s[20];
  int sum1;
  bool a=0;
  sscanf(args, "%s", s);
//	printf("%s\n",args);
  sum1 =expr(s,&a);
  printf("ans:%d",sum1);
	return 0;
}
static int cmd_w(char* args) {
	TestCorrect(args == NULL);
	bool flag = true;
	uint32_t v = expr(args, &flag);
	TestCorrect(!flag);
	WP *wp = new_wp();
	if (wp == NULL) {
		printf("No space to add an extra watchpoint!");
		return 0;
	}
	strcpy(wp -> exprs, args);
	wp -> val = v;
	printf("Succefully add watchpoint NO.%d\n", wp -> NO);
	return 0;
}
static int cmd_d(char* args) {
	TestCorrect(args == NULL);
	int id;
	sscanf(args, "%d", &id);
	bool flag = true;
	WP* wp = delete_wp(id, &flag);
	if (!flag) {
		printf("Cannot Find!\n");
		return 0;
	}
	free_wp(wp);
	printf("Succefully Delete!\n");
	return 0;
}
static int cmd_q(char *args) {
  return 0;
}
static int cmd_info(char *args) {
  char *arg=strtok(NULL," ");  
    if(strcmp(arg,"r") == 0){  
       isa_reg_display();
    }  
     else if (args[0] == 'w') {
		print_w();
	}
    return 0;  
}

static int cmd_x(char *args){
    //获取内存起始地址和扫描长度。
    if(args == NULL){
        printf("too few parameter! \n");
        return 1;
    }
     
    char *arg = strtok(args," ");
    if(arg == NULL){
        printf("too few parameter! \n");
        return 1;
    }
    int  n = atoi(arg);
    char *EXPR = strtok(NULL," ");
    if(EXPR == NULL){                                                                                                                                          
        printf("too few parameter! \n");
        return 1;
    }
    if(strtok(NULL," ")!=NULL){
        printf("too many parameter! \n");
        return 1;
    }
    bool success = true;
    //vaddr_t addr = expr(EXPR , &success);
    if (success!=true){
        printf("ERRO!!\n");
        return 1;
    }
    char *str;
   // vaddr_t addr = atoi(EXPR);
    vaddr_t addr =  strtol( EXPR,&str,16 );
   // printf("%#lX\n",ad);
    //进行内存扫描,每次四个字节;
    for(int i = 0 ; i < n ; i++){
        uint32_t data = vaddr_read(addr + i * 4,4);
        printf("0x%08x  " , addr + i * 4 );
        for(int j =0 ; j < 4 ; j++){
            printf("0x%02x " , data & 0xff);
            data = data >> 8 ;
        }
        printf("\n");
    }
     
    return 0;
}   


/*
static int cmd_x(char *args){  
    char *N = strtok(NULL," ");  
    char *EXPR = strtok(NULL," ");  
    int len;  
    vaddr_t address;  
      
    sscanf(N, "%d", &len);  
    sscanf(EXPR, "%x", &address);  
      
    printf("0x%x:",address);  
    int i;
    for(i = 0; i < len; i ++){  
        printf("%08x ",vaddr_read(address,4));  
        address += 4;  
    }  
    printf("\n");  
    return 0;  
}
*/



static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "单步执行", cmd_si },
  { "info","打印程序状态",cmd_info},
  { "x","扫描内存",cmd_x  },
  { "p","表达式求值",cmd_p },
  { "w", "Add watchpoint", cmd_w},
	{ "d", "Delete watchpoint", cmd_d
  },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}


 
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

  
