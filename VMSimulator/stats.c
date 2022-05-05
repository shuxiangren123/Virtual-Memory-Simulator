/*
	统计数据
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <stats.h>
#include <options.h>

stats_t *stats;

void stats_output_type(FILE *o, type_count_t output, const char *label);
void stats_ratio_hit(FILE* o, type_count_t output1, type_count_t output2, const char *label);

void stats_init() {
  	stats = (stats_t*)calloc(1, sizeof(stats_t));//在内存的动态存储区中分配num个长度为size的连续空间，函数返回一个指向分配起始地址的指针；如果分配不成功，返回NULL。
  	assert(stats);//申请失败则推出
  	if (opts.output_file) {
    		stats->output = fopen(opts.output_file, "a+");
    	if (stats->output == NULL) {
      		perror("无法打开输出文件进行写入");
      		abort();//结束
    	}
  	} 
  	else {
    		stats->output = stdout;//正常输出
  	}
}

void stats_output() {
  	FILE *o = stats->output;
  	fprintf(o, "\n\n 模拟参数:"); 
  	fprintf(o, "\n    物理大小（页面数）:%d, 页面大小:%d, 输入文件:%s, 替换算法:%s, ref_limit:%ld\n",opts.phys_pages, opts.pagesize,
	  (opts.input_file ? opts.input_file : "stdin"),
	  opts.fault_handler->name, opts.limit);

  
  	fprintf(o, "\n 模拟结果:"); 
  	fprintf(o, "\n\t 统计类型 : 	code,	load,	store;   	total\n");
  	stats_output_type(o, stats->references, " 内存引用	");
  	stats_output_type(o, stats->miss, " 页面缺失	");
  	stats_output_type(o, stats->compulsory, " 首次页面缺失	");
  	stats_output_type(o, stats->evict_dirty, "（脏）页面写入	");
	
	stats_ratio_hit(o, stats->miss, stats->references, " 命中率		");
  	fclose(o);
  	stats->output = NULL;
}

void stats_output_type(FILE* o, type_count_t output, const char *label) {
  	fprintf(o, "\t%s: %u,	%u,	%u;  		%u\n", label, output[REF_KIND_CODE], output[REF_KIND_LOAD], output[REF_KIND_STORE], 
	output[REF_KIND_CODE] + output[REF_KIND_LOAD] + output[REF_KIND_STORE]);
}

void stats_ratio_hit(FILE* o, type_count_t output1, type_count_t output2, const char *label){
	fprintf(o, "\t%s: %.4f%c\n", label, (1.0-(float)output1[REF_KIND_LOAD]/output2[REF_KIND_LOAD])*100.0,'%');
}
