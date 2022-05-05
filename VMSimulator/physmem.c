/*
	定义物理内存样式
*/
#include <assert.h>
#include <stdlib.h>

#include <options.h>
#include <pagetable.h>
#include <physmem.h>
#include <stats.h>

pte_t **physmem;

void physmem_init() {
  	physmem = (pte_t**)(calloc(opts.phys_pages, sizeof(pte_t*)));
  	assert(physmem);//如果申请失败则抛出错误结束程序
}

pte_t **physmem_array() {
  	return physmem;
}

void physmem_evict(uint pfn, ref_kind_t type) {//对页面进行换出
  	assert(0 <= pfn && pfn < opts.phys_pages);//错误检查，虚拟页号要大于0且物理页号小于总页面数

  	/* 无页面，啥也不做*/
  	if(physmem[pfn] == NULL || !physmem[pfn]->valid) {
    		physmem[pfn] = NULL;
    		return;
  	}

#ifdef DEBUG
  	printf("移出页面 ： pfn=0x%x 到磁盘\n", pfn);//调试模式
#endif
  	stats_evict(type);//换出次数加一
  	if (physmem[pfn]->modified) {//如果被修改过，记脏位加一
    		stats_evict_dirty(type);
  	}
  	physmem[pfn]->frequency=0;
  	physmem[pfn]->modified = 0;
  	physmem[pfn]->valid = 0;
  	physmem[pfn] = NULL;
}

void physmem_load(uint pfn, pte_t *new_page, ref_kind_t type) {
  	assert(0 <= pfn && pfn < opts.phys_pages);//合法性检测
  	assert(new_page && !new_page->valid);//有效位也要为1
  	assert(physmem[pfn] == NULL);

  	physmem[pfn] = new_page;

  	physmem[pfn]->pfn = pfn;
  	physmem[pfn]->reference = 0;
  	physmem[pfn]->modified = 0;
  	physmem[pfn]->valid = 1;

}

void physmem_dump() {
  	uint i;

  	printf("\n当前物理内存 pte 字段.\t有效位  vfn  \tpfn         修改位        reference        counter       ResetCounter(c)          frequency\n");

  	for(i = 0 ; i < opts.phys_pages; i++) {
                 pte_t *pte=(pte_t *) physmem[i];
		 if (pte) {
                 	printf("physmem[0x%x]: \t\t%d  \t0x%x  \t0x%x  \t\t%d  \t\t%d \t\t%d \t\t %d \t\t      %d\n",
                          i,
                          pte->valid,
                          pte->vfn,
                          pte->pfn,
                          pte->modified,
                          pte->reference,
			  pte->counter,
                          pte->c,
			  pte->frequency);
		}
  	}
}
