/*
 	physmem 模块对物理内存页面进行建模.
 */

#ifndef PHYSMEM_H
#define PHYSMEM_H

#include <vmsim.h>
#include <pagetable.h>

/* 将物理内存初始化为全空. */
void physmem_init();

/* 获取代表物理内存的 pte_ts 数组。
  * 不要直接修改这个数组以及它的元素，
  * 使用 physmem_evict/physmem_load。
  * 数组中有 opts.phys_pages 元素。
  * 空元素为 NULL。 */
pte_t** physmem_array();

/* 从内存中移除给定 pfn 的页面。 类型应指定
  * 导致驱逐的引用类型（即传递的类型到故障处理程序）。 将 pfn 标记为空（适用于physmem_load）。 */
void physmem_evict(uint pfn, ref_kind_t type);

/* 将给定的页面 (pte) 加载到给定的物理内存插槽 (pfn) 中。
  * 该插槽应该是空的（要么因为它从未被使用过，要么
  * 因为那里的页面已被驱逐）。 类型应该指定什么
  * 类型的参考造成了负载。*/
void physmem_load(uint pfn, pte_t *pte, ref_kind_t type);

void physmem_dump();
extern pte_t **physmem;

#endif 
