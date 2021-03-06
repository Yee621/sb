#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_b(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step execution", cmd_si },
  { "info", "Print register state", cmd_info },
  { "x", "Print the address", cmd_x },
  { "p", "Expression evaluation", cmd_p},
  { "w", "Info the watchpoints and breakpoints or create a new watchpoint", cmd_w },
  { "d", "Delete a watchpoint", cmd_d },
  { "b", "Create a new breakpoint", cmd_b},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

static int cmd_si(char *args) 
{
	int step_num;
	if(args==NULL) 
	{
		cpu_exec(1);
	}
	else 
	{
		step_num=atoi(args);
		if(step_num>=15)
		{
			printf("The number of 'si' must be smaller than 15!\n");
		}
		else if(step_num<-1)
		{
			printf("The number of 'si' must be bigger than -2!\n");
		}
		else if(step_num==0)
		{
			cpu_exec(1);
		}
		else
		{
			cpu_exec(step_num);
		}
	}
	return 0;
}

static int cmd_info(char *args)
{
	int i;
	if(args==NULL)
	{
		printf("You haven't print the args!\n");
	}
	else if(strcmp(args,"r")==0)
	{
		for(i=0;i<8;i++)
		{
			printf("%s	0x%08x	%d\n",regsl[i],reg_l(i),reg_l(i));
		}
		for(i=0;i<8;i++)
		{
			printf("%s	0x%08x	%d\n",regsw[i],reg_w(i),reg_w(i));
		}
		for(i=0;i<8;i++)
		{
			printf("%s	0x%08x	%d\n",regsb[i],reg_b(i),reg_b(i));
		}
	}
	return 0;
}

static int cmd_x(char *args)
{
	char *str_1,*str_2;
	int i,num;
	uint32_t addr;
	str_1=strtok(args," ");
	str_2=strtok(NULL," ");
	num=atoi(str_1);
	if(num<1)
	{
		printf("The number must be bigger than 0!\n");
	}
	else
	{
		sscanf(str_2,"%x",&addr);
		for(i=0;i<num*4;i+=4)
		{
			printf("0x%x	%02x %02x %02x %02x\n",addr+i,vaddr_read(addr+i,1),vaddr_read(addr+i+1,1),vaddr_read(addr+i+2,1),vaddr_read(addr+i+3,1));
		}
	}
	return 0;
}

static int cmd_p(char *args)
{
	bool success=true;
	uint32_t result;
	result=expr(args,&success);
	if(success==true)
		printf("result: %d\n",result);
	return 0;
}

static int cmd_w(char *args)
{
	if(args==NULL)
	{
		info_wp();
	}
	else
	{
		new_wp(args);
	}
	return 0;
}

static int cmd_d(char *args)
{
	if(args==NULL)
	{
		printf("You haven't input the number of watchpoint to delete!\n");
	}
	else
	{
		int num;
		sscanf(args,"%d",&num);
		free_wp(num);
	}
	return 0;
}

static int cmd_b(char *args)
{
	if(args==NULL)
	{
		printf("You haven't input the expression of the breakpoint!\n");
	}
	else
	{
		new_bp(args);
	}
	return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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
