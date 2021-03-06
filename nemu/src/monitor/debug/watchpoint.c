#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
   for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

void  new_wp(char *args)
{
	if(strlen(args)>31)
		panic("The expression is too long!");
	if(free_!=NULL)
	{
		WP *p;
		bool success=true;
		p=free_;
		free_=free_->next;
		memset(p->expr,0,sizeof(p->expr));
		strcpy(p->expr,args);
		p->new_value=expr(args,&success);
		p->type='w';
		if(head==NULL)
		{
			head=p;
		}	
		set_NO();
	}
	else
	{
		printf("The watchpoint has been used up!");
		assert(0);
	}
}

void  new_bp(char *args)
{
	if(strlen(args)>31)
		panic("The expression is too long!");
	if(free_!=NULL)
	{ 
		WP *p;
		bool success=true;
		p=free_;
		free_=free_->next;
		memset(p->expr,0,sizeof(p->expr));
		strcpy(p->expr,args);
		p->new_value=expr(args,&success);
		p->type='b';
		if(head==NULL)
		{
			head=p;
	 	}
		set_NO();
	}
	else
	{
		printf("The breakpoint has been used up!");
		assert(0);
	}
} 

void free_wp(int num)
{
	if(head==NULL)
		panic("There is no watchpoint!");
	WP *p,*q;
	p=head;
	q=NULL;
	if(p->NO==num)
	{
		q=p;
		head=p->next;
	}
	else
	{
		for(p=p->next;p->next!=free_;p=p->next)
		{
			if(p->next->NO==num)
			{
				q=p->next;
				p->next=p->next->next;
				break;
			}
		}
	}
	if(q==NULL)
		panic("Can't find the NO.%d wp!",num);
	q->NO=0;
	q->new_value=0;
	q->next=free_->next;
	free_->next=q;
	set_NO();
}

void set_NO()
{
	WP *p;
	int num=1;
	for(p=head;p!=free_;p=p->next)
	{
		p->NO=num;
		num++;
	} 
}

int check_wp()
{
	WP *p=head;
	if(p!=NULL)
	{
		p=head;
		int change=0;
		uint32_t value;
		bool success=true;
		while(p!=free_)
		{
			if(p->type=='w')
 			{
				value=expr(p->expr,&success);
				if(value!=p->new_value)
 				{
					printf("watchponit %s:\n",p->expr);
					printf("   old_value: %x\n",p->new_value);
					printf("   new_value: %x\n",value);
					change=1;
					p->old_value=p->new_value;
					p->new_value=value;
				} 
			}
			p=p->next;
		}
		return change;
	}
	return 0;
}

int check_bp()
{
	WP *p=head;
	if(p!=NULL)
	{
		p=head;
		int change=0;
		uint32_t value;
		bool success=true;
		while(p!=free_)
		 {
			if(p->type=='b')
		 	{
				change=2;
				value=expr(p->expr,&success);
				if(value!=p->new_value)
		 		{
					printf("breakpoint %s:\n",p->expr);
					printf("   old_value: %x\n",p->new_value);
					printf("   new_value: %x\n",value);
					p->old_value=p->new_value;
					p->new_value=value;
					change=1;
				}
				break;
		 	}
			p=p->next;
		}
		if(change==2)
		{
			printf("There is no change at the breakpoint %s!\n",p->expr);
			change=1;
		}
		return change;
	}
	return 0;
}

void info_wp()
{
	WP *p;
	p=head;
	if(p!=NULL)
		printf("Num	type	expr	value\n");
	else 
		printf("There is no watchpoint or breakpoint!\n");
	while(p!=free_)
	{
		printf("%d	%c	%s	%x\n",p->NO,p->type,p->expr,p->new_value);
		p=p->next;
	}
}

/* TODO: Implement the functionality of watchpoint */


