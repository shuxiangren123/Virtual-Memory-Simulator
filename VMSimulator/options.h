#ifndef OPTIONS_H
#define OPTIONS_H

#include <vmsim.h>
#include <fault.h>

typedef struct _opts {
  	bool_t verbose;//详细
  	bool_t test;//测试
  	bool_t addr_create;//选择此项生成一些随机地址流
  	int pagesize;
  	int phys_pages;
  	long limit;
  	char *output_file;
  	char *input_file;
  	fault_handler_info_t *fault_handler;
} opts_t;

extern opts_t opts;

void options_process(int argc, char **argv);

//#define DEBUG
#endif
