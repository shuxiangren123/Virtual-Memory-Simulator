/*
	声明可用的故障处理程序.
*/
#ifndef FAULT_H
#define FAULT_H

#include <unistd.h>
#include <vmsim.h>
#include <pagetable.h>

/* fault handlers 是无返回值函数且
  有两个参数: a pte_t* and a ref_kind_t.
  pte是必须插入的新页, 
  type 被用于统计数据. */
typedef void (*fault_handler_t)(pte_t *pte, ref_kind_t type);

// Fault_handler_info_t将实际函数与名称匹配起来。 通过options.c搜索Fault_handlers以找到命令行中指定的处理程序.
typedef struct fault_handler_info {
  	char *name;
  	fault_handler_t handler;
} fault_handler_info_t;

extern fault_handler_info_t fault_handlers[];//定义一个结构体数组外部声明，每个数组项内含一个名称以及一个函数指针同义字
//用于在options中打印所有算法名

#endif 
