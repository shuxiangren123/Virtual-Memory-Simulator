#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <vmsim.h>

//可从命令行重写的默认值
const static int pagesize = 4096;
const static int log_pagesize = 12;

typedef struct pte {
  	uint           vfn; // 虚拟页号 
  	uint           pfn; // 物理页号 iff valid=1 
  	int           reference;
  	bool_t        valid; // 在physmem为真, 其他为假
  	bool_t        modified;
  	int 		counter;  // 用于 LRU, FIFO，实现于LRU
  	int		frequency; //用于 LFU 和 MFU 
  	int 		c; //实现于 FIFO ， LFU 和 MFU 相关顺序
  	int		used; //时钟算法使用位
  	int           chance; // second chance algorithm算法的修改位	

} pte_t;

typedef struct _pagetable_level{

  	uint size;		//总大小
  	uint log_size;	//总大小的2的对数
  	bool_t is_leaf;	
  
} pagetable_level_t;

/* 此结构体表示多级页表 */
typedef struct _pagetable {
  	void **table; // 二维数组，如果是最低级别，则为 pte_t 指针数组；否则为 pagetable_t 指针数组。 
  	int level;
} pagetable_t;

void pagetable_init();

/* 查找给定虚拟页的页表条目.
 * 如果页表不在内存, 使得valid==0.
 * 如果vfn 从未见到过, 将创造一个新的pte_t
 * 以及赋予一个vfn 和 valid==0.
 * 类型用于统计跟踪.
 */
pte_t *pagetable_lookup_vaddr(uint vfn, ref_kind_t type);

void pagetable_test();

void pagetable_dump();
#endif /* PAGETABLE_H */
