/*
	为虚拟页表建模;用于每一个引用查找pte_t并确认它是否在内存中。然而，2级表和优化的1级表表似乎有同样的性能。
 */

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include <vmsim.h>
#include <util.h>
#include <options.h>
#include <pagetable.h>
#include <stats.h>



/* 定义多级页表.
 * 这定义了每一层的最大值.
 * Pagetable init可以选择减小大小,
 * 如果需要更少的位(因为页面大小更大)。. */
 
 //uint size;		//总大小	，uint log_size;	//总大小的2的对数	，bool_t is_leaf;
pagetable_level_t levels[3] = {
  { 4096, 12, FALSE }, 	//levels[0] 有12位，4096的大小. 如果 vfn_bits 超过 12, 就需要额外的levels
  { 4096, 12, FALSE }, 	//levels[1] 有 12 位. 如果 vfn_bits 超过 24, 就需要额外的 levels
  { 256, 8, TRUE } 		//levels[2] 有 8 位. 所以最大的可被处理的 vfn_bits 是 12+12+8=32 
};

uint vfn_bits;//vfn_bits是虚帧号中的位数,vfn_bits应该是所有级别log_size字段的和。也就是多级页表的总位数不能超出总页号位数


/*now_table ->table 是当前页表。使用pte_t *pte=(pte_t *) now_table->table[i]访问每个pte表项*/
static pagetable_t *now_table;  

pte_t *pagetable_lookup_helper(uint vfn, uint bits, uint masked_vfn, pagetable_t *pages, ref_kind_t type);
pte_t *pagetable_new_pte(uint vfn);
pagetable_t *pagetable_new_table(int level);

inline uint getbits(uint x, int p, int n);

void pagetable_init() {
  
	uint page_bits;
	page_bits = log_2(opts.pagesize);//对2求对数，在util中实现
  
	if (page_bits == -1) {
    	fprintf(stderr, "vmsim: 页面大小必须是2的幂\n");
    	abort();//不是就退出
  	}
  	vfn_bits = addr_space_bits - page_bits;//虚拟页号为总地址空间减去页内偏移
  	//32的块大小，vfn_bits=11

  	uint bits = 0;
  	int level = 0;
  	while (1) {
    		bits += levels[level].log_size;//第一次就是0+12=12
    		if (bits >= vfn_bits)//12>=11?,例如在32情况时是满足的。
    			//当块内小，vfn_bits大时，如块内为8，vfn_bits=16-3=13，12>=13不成立就要加一级level，下一级level再加12
      			break;
    		level++;
  	}//累积申请一个最小的合适的bits大小，最大12+12+8

  	//对多级页表的每一级进行计算，只需重新计算level所在一级，前面的结构体内部变量已满，与初始设置一致，故从二级也表开始此处才有意义
  	levels[level].log_size = levels[level].log_size - (bits - vfn_bits);//为最高几位页表号的位数，二级最大log_size为12. 
  	levels[level].size = pow_2(levels[level].log_size);//位数对应大小，为2的幂次，在tuil中实现
  	levels[level].is_leaf = TRUE;//表示被启用了
  
  	if (opts.test) {//在测试模式下看看分级成不成功
    		int i;
    		printf("vmsim: vfn_bits %d, %d 级页表\n", vfn_bits, level+1);//level数加一为页表级数
    		for (i=0; i<=level; i++) {
      			printf(" 第 %d级: %u 位大小 (%u 输入)\n", i, levels[i].log_size,levels[i].size);
    		}
  	}
  
  	now_table = pagetable_new_table(0);//创建table所需空间并返回首地址
}

pagetable_t *pagetable_new_table(int level) {
  
  	pagetable_t *table;
  	pagetable_level_t *config;
  
  	config = &levels[level];//第level级页表的数组地址
 
  	table = malloc(sizeof(struct _pagetable));
  	assert(table);//看看是否申请成功

  	table->table = calloc(config->size, sizeof(void*));//为二维数组申请空间
  	//在动态分配完内存后，自动初始化该内存空间为零。两个参数的乘积就是要分配的内存空间的大小
  	assert(table->table);//同上

  	table->level = level;

  	return table;//返回首地址
}

pte_t *pagetable_lookup_vaddr(uint vfn, ref_kind_t type) {
  	return pagetable_lookup_helper(vfn, 0, vfn, now_table, type);
}

/* 递归搜索页表. 
  创建搜索中缺失的任何条目(无论是页表级别还是pte_t本身)。 返回给定vfn处的pte_t(如果之前没有vfn，则返回一个新的vfn) 
  vfn  虚拟页号
  masked_vfn 屏蔽分配给高级页表的位后的虚拟帧号
  bits  已被更高级别视为的(高)比特数。
  pages  本级别的页表。
  对于单级页表，index=masked_vfn=vfn。Getbits()只返回它的第一个参数。
 */
pte_t *pagetable_lookup_helper(uint vfn, uint bits, uint masked_vfn,	pagetable_t *pages, ref_kind_t type) {
  	uint index;
  	int log_size;

  	log_size = levels[pages->level].log_size;

  	index = getbits(masked_vfn, vfn_bits - (1+bits), log_size);
  
  	bits += log_size;
  	masked_vfn = getbits(masked_vfn, vfn_bits - (1+bits), vfn_bits - bits);
  
  	if (levels[pages->level].is_leaf) {
    		if (pages->table[index] == NULL) {
      			//强制缺失 - 首次访问
      			stats_compulsory(type);
      			pages->table[index] = (void*)pagetable_new_pte(vfn);
   		}
    		return (pte_t*)(pages->table[index]);
  	} 
  	else {
    		if (pages->table[index] == NULL) {
     	 		pages->table[index] = pagetable_new_table(pages->level+1);
    		}
    		return pagetable_lookup_helper(vfn, bits, masked_vfn, pages->table[index], type);
  	}
}

pte_t *pagetable_new_pte(uint vfn) {
  	pte_t *pte;
  	pte = (pte_t*)(malloc(sizeof(pte_t)));
  	assert(pte);

  	pte->vfn = vfn;
  	pte->pfn = -1;
  	pte->valid = FALSE;
  	pte->modified = FALSE;
  	pte->reference = 0;

  	return pte;
}

void pagetable_test() {//测试计算是否正确
  	printf("测试 pagetables\n");
  	pagetable_init();
  	assert(now_table);
}

void pagetable_dump() {
  	assert(now_table);
  	assert(now_table->level==0);
  	uint vfn_bits=addr_space_bits-log_2(opts.pagesize);
  	/*page_bits = log_2(opts.pagesize);
  	if (page_bits == -1) {
    	fprintf(stderr, "vmsim: Pagesize must be a power of 2\n");
    	abort();
  	}
  	vfn_bits = addr_space_bits - page_bits;
  	*/
  	uint pt_size=pow_2(vfn_bits);
  	printf("\n当前页表pte字段.        有效位     vfn\t\tpfn     修改位     reference     counter\t\n");
  	uint i;
  	for(i=0; i< pt_size;i++) {
	  	if(now_table->table[i]) {
		  	pte_t *pte=(pte_t *) now_table->table[i];
		  	printf("table[0x%x]:\t         %d        0x%x\t\t0x%x        %d             %d           %d\n",  
			  	i,
			  	pte->valid,
			  	pte->vfn,
			  	pte->pfn,
			  	pte->modified,
			  	pte->reference,
			  	pte->counter
			  	);
	  	}
  	}
}
