#ifndef UTIL_H

#define UTIL_H

#include <sys/types.h>
#include <vmsim.h>

//从位置 p 获取 n 位
static inline uint getbits(uint x, int p, int n) {
  	return (x >> (p+1-n)) & ~(~0 << n);
}

static inline uint vaddr_to_vfn(vaddr_t vaddress) {
  	return getbits(vaddress, addr_space_bits-1, vfn_bits);
}

uint log_2(uint x);
uint pow_2(uint pow);

void util_test();

#endif
