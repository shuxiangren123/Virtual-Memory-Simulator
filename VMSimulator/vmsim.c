/*
	主运行文件
*/
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>	//异常检测

#include <vmsim.h>
#include <util.h>
#include <options.h>
#include <pagetable.h>
#include <physmem.h>
#include <stats.h>
#include <fault.h>

void init();
void test();
void simulate();

uint dot_interval = 100;
uint dots_per_line = 64;

int ref_counter = 0;//参考次数计数器,用于pte->counter
int r=0;//条目计数器，用于pte->c

int main(int argc, char **argv) {
	options_process(argc, argv);
	if (opts.test) {
		printf("正在测试……\n");
    		test();
    		printf("测试完成.\n");
    		exit(0);
  	}
  	init();		//各项初始化
  	simulate();		//模拟
  	stats_output();	//统计输出
	return 0;
}

void init() {   
	
	pagetable_init();//页表初始化
	physmem_init();//物理内存初始化
	stats_init();//统计数据初始化
}



void test() {//当输入-t时，测试几个函数
	printf("运行测试...\n");
	util_test();
	stats_init();
	pagetable_test();
}

ref_kind_t get_type(char c){
	if (c == 'R') 
		return REF_KIND_LOAD;//为加载（读取）类型
	if (c == 'W') 
		return REF_KIND_STORE;//为写入类型
	return REF_KIND_CODE;//啥也不是
}//判断是读还是写，返回值为枚举类型


void simulate() {
	uint pid;//进程号
	char ch;//R 或 W？
	vaddr_t vaddr;//虚拟内存地址
	ref_kind_t type;//读写类型
	pte_t *pte;
	fault_handler_t handler;//初始化错误处理函数，即typedef void (*fault_handler_t)(pte_t *pte, ref_kind_t type);
	uint count = 0;//统计输入个数
	FILE *fin = NULL;	
#ifdef DEBUG
	char choice[5];//接收输入,y/n
	uint pgfault=FALSE;
#endif
  
	handler = opts.fault_handler->handler;
  
	if ((fin=fopen(opts.input_file, "r")) == NULL) {//此处根据输入打开文件
		fprintf(stderr, "\n 无法打开文件 %s.", opts.input_file);
		exit(1);
  	}
 #ifdef DEBUG
   	printf("\n\n开始模拟: ");
  	printf("vaddr (虚拟地址) 有 %d bits, 连续的高 %d bits 是 vfn (虚拟页号), 低 %d bits 为页内偏移 (log_2(pagesize=%d))\n",
	addr_space_bits, vfn_bits, log_2(opts.pagesize), opts.pagesize);
#endif
  	while (fscanf(fin, "%d, %c, %x", &pid, &ch, &vaddr) !=EOF) {//文件形式输入
		type = get_type(ch);//返回读写类型枚举类，即0，1或2
	  	stats_reference(type);
	  	count++;
    
    		pte = pagetable_lookup_vaddr(vaddr_to_vfn(vaddr), type);//对虚拟地址进行搜索,指向命中的那个
#ifdef DEBUG
    		printf("_______________________________________________________________\n");
    		printf("\n获得 count=%d 存储访问， pid:%d mode:%c vaddr:0x%08x(当前文件) vfn:0x%x\n",
			count, pid, ch, vaddr, vaddr_to_vfn(vaddr));
      		pgfault=!pte->valid;
      		printf("\n此页面 %s. 是否要转储页表和物理内存？ y or n: ", pgfault? "未命中":"命中");
      		scanf("%s", choice);
#endif
    		if (!pte->valid) { //未命中
      			stats_miss(type);//统计该种类型次数
      
      			/***********************/
      			handler(pte, type);//处理这种类型问题,此为核心
      			/************************/
	  
			pte->c=r++;//该数组未命中次数
    		} 

    		if(pte->valid) //命中！为了 LFU 和 MFU , "chance"被修改，为了第二次机会算法
    		{	
			pte->frequency = pte->frequency + 1;	//使用频次标记，lfu和mfu
			pte->used = 1;		//使用位，时钟替换
			pte->chance = 1;	//机会算法标记
    		}
    
    		pte->reference = 1;
    		pte->counter = ref_counter++; //LRU使用,先标记一下使用时的数，再增加
    						//无论命不命中，都在刚使用的那个物理位置标上一个数

    		if (type == REF_KIND_STORE)
      			pte->modified = TRUE;

#ifdef DEBUG
      		if (choice[0]=='Y' || choice[0]=='y') {//增加程序容错率
			pagetable_dump();
			physmem_dump();
			choice[0]='N';//默认就是N，防止乱输入
      		}
#endif
    		if (opts.limit && count >= opts.limit) {
      			if (opts.verbose)
				printf("\nvmsim: reached %d references\n", count);
      			break;
    		}

  	}
}

