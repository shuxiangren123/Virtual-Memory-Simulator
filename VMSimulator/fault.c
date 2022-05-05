/*
	定义可用的错误处理程序.
*/

#include <vmsim.h>
#include <fault.h>
#include <options.h>
#include <physmem.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void fault_random(pte_t *pte, ref_kind_t type);
static void fault_lfu(pte_t *pte, ref_kind_t type);
static void fault_mfu(pte_t *pte, ref_kind_t type);
static void fault_lru(pte_t *pte, ref_kind_t type);
static void fault_fifo(pte_t *pte, ref_kind_t type);
static void fault_clock(pte_t *pte, ref_kind_t type);
static void fault_second(pte_t *pte, ref_kind_t type);

fault_handler_info_t fault_handlers[8] = {
  	{ "random", fault_random },
  	{ "lfu", fault_lfu },
  	{ "lru", fault_lru },
  	{ "fifo", fault_fifo },
  	{ "mfu", fault_mfu },
  	{ "clock", fault_clock },
  	{ "second", fault_second }, 
  	{ NULL, NULL } /* 最后一个条目必须是NULL/NULL */
};


//随机替换算法
void fault_random(pte_t *pte, ref_kind_t type) {
  
	srand((unsigned)time(NULL));	//使用时间种子产生随机数
  
  	int page;
  	page = random() % opts.phys_pages;//随机选择一个丢掉，生成一个在页面内的余数
  	physmem_evict(page, type);//换出
  	physmem_load(page, pte, type);//同位放入
}

// LRU 替换算法
void fault_lru(pte_t *pte, ref_kind_t type) {

	static int frame = 0;//从0开始的物理页号

	int i,location=0;
	int minimum=0;

	static int physmem_occupied = 1;//当前物理页面占用数

	if(physmem_occupied <= opts.phys_pages)//判断当前物理页面是否已满
	{
		physmem_load(frame, pte, type);//physmem_load(uint pfn, pte_t *new_page, ref_kind_t type);
		frame++;//顺序排页框号，其实也可以其他顺序
		physmem_occupied++;			

	}

	else
	{
		
		minimum = 65535;
		for(i=0;i<opts.phys_pages;i++)//遍历物理内存
		{
			if(minimum > physmem[i]->counter)//找到最小那个
			{
				minimum = physmem[i]->counter;//LRU/FIFO专用计数器
				location = i;
			} 
		}
		#ifdef DEBUG
		printf("\n根据LRU规则，");
		#endif
		physmem_evict(location, type);		
		physmem_load(location, pte, type);

	

	}


}

// FIFO 替换算法
void fault_fifo(pte_t *pte, ref_kind_t type) {

  	int i,location=0,min;
	static int frame = 0;//从0开始的物理页号
	static int physmem_occupied = 1;//当前物理页面占用数

	if(physmem_occupied <= opts.phys_pages)
	{
		physmem_load(frame, pte, type);
		frame = frame + 1;
		physmem_occupied = physmem_occupied + 1;
	}//没满先放

	else
	{
		min = physmem[0]->c;//定义在physmem.h中的pte类型二维数组
		for(i = 0;i<opts.phys_pages;i++)
		{

			if(physmem[i]->c < min)
			{
				min = physmem[i]->c;
				location = i;	
			}

		}//找出fifo标记最小的，也就是最早放进去的
#ifdef DEBUG
	printf("\n根据FIFO规则，");
#endif
	physmem_evict(location, type);		
	physmem_load(location, pte, type);

	}
}
//最少使用算法 LFU
void fault_lfu(pte_t *pte, ref_kind_t type) {

	static int frame = 0;
	static int physmem_occupied = 1;
	
	int i,location = 0;;

	pte_t *mark=(pte_t*)malloc(sizeof(pte_t)); 

		
	//未装满
	if(physmem_occupied <= opts.phys_pages)
	{
		physmem_load(frame, pte, type);
		frame++;
		physmem_occupied++;			

	}

	else
	{
		mark->frequency = physmem[0]->frequency;//随便赋一个值，去寻找最小值
		for(i=0;i<opts.phys_pages;i++)//遍历物理内存
		{

			if(mark->frequency > physmem[i]->frequency)
			{
				mark->frequency = physmem[i]->frequency;
				location = i;
				mark->c = physmem[i]->c;//处理使用次数相同时，fifo标记位
			}

			
		}

		//保持先进先出顺序
		for(i=0;i<opts.phys_pages;i++)
		{
			if((physmem[i]->frequency == mark->frequency) && (physmem[i]->c < mark->c))
				location = i;
		}

		physmem_evict(location, type);
  		physmem_load(location, pte, type);
		
	
	}	


}



//最常使用的算法 MFU
void fault_mfu(pte_t *pte, ref_kind_t type) {


	static int frame = 0;
	
	int i;

	pte_t *test=(pte_t*)malloc(sizeof(pte_t));

	
	int location = 0;

	static int physmem_occupied = 1;
	
	if(physmem_occupied <= opts.phys_pages)
	{
		physmem_load(frame, pte, type);		
		frame = frame + 1;
		physmem_occupied = physmem_occupied + 1;			

	}//没放满先继续放

	else
	{

		
		test->frequency = physmem[0]->frequency;
		for(i=0;i<opts.phys_pages;i++)
		{

			if(test->frequency < physmem[i]->frequency)//寻找frequency最大的那个
			{
				test->frequency = physmem[i]->frequency;
				location = i;
				test->c = physmem[i]->c;
			}

			
		}

		
		for(i=0;i<opts.phys_pages;i++)
		{
			if((physmem[i]->frequency == test->frequency) && (physmem[i]->c < test->c))
				location = i;
		}

		physmem_evict(location, type);
  		physmem_load(location, pte, type);
		
	
	}	

}

//Clock 替换算法
static void fault_clock(pte_t *pte, ref_kind_t type) {

	static int physmem_occupied = 1;
	static int frame = 0;
	int i,location=0;
	static int clock_mark = 0;

	if(physmem_occupied <= opts.phys_pages)
	{
		physmem_load(frame, pte, type);
		physmem[frame]->used = 1;	//使用位置为1
		frame++;
		physmem_occupied++;
					

	}

	else
	{
		for(i=clock_mark;i<opts.phys_pages;i++)
		{
			if(physmem[i]->used == 0)
			{
				location = i;
				if((i + 1) == opts.phys_pages)//这是表尾
					clock_mark = 0;//clock标记位从0，表头开始
				else
					clock_mark = ++i;//否则就从当前换出的下一个位置开始
				break;
			}//未被用过就直接换出，这是唯一退出循环的方式
			
			else
			{
				physmem[i]->used = 0;
			}//被用过就标记为0，下次就可换出了

			if((i+1) == opts.phys_pages )//到最后一个了,就把指针放回原点，以便于用来循环。由于还要自增1，就变为-1
				i = -1;	


		}

				
		physmem_evict(location, type);  
		physmem_load(location, pte, type);

		//clock_mark++;

		//if(clock_mark == opts.phys_pages)
			//clock_mark = 0;
		

		
		
	}


}


//第二次机会算法  FIFO变体
static void fault_second(pte_t *pte, ref_kind_t type)
{
	static int physmem_occupied = 1;
	static int frame = 0;
	int i,j;
	int location = 0;	//此处初始化为0，当所有页面的chance都为1时，就将最开始的0号换出
	//int loc2=0,min=0;

	//pte_t *temp =(pte_t *)malloc(sizeof(pte_t));
	pte_t *temp;
	//先装
	if(physmem_occupied <= opts.phys_pages)
	{
		physmem_load(frame, pte, type);
		frame = frame + 1;
		physmem_occupied = physmem_occupied + 1;
	}

	//找最小值
	
	else
	{
		for(i=0;i<opts.phys_pages;i++)
		{
			for(j=0;j<(opts.phys_pages-i-1);j++)
			{
				if(physmem[j+1]->c < physmem[j]->c)
				{
					temp = physmem[j];
					physmem[j] = physmem[j+1];
					physmem[j+1] = temp;
				}
			}
		}


		for(i=0;i<opts.phys_pages;i++)
		{
			if(physmem[i]->chance == 1)
			{
				physmem[i]->chance = 0;
 
			}

			else 
			{
				location = i;
				break;
			}

		}

		
		physmem_evict(location, type);  
		physmem_load(location, pte, type);


	}


}



