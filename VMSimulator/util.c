/*
	小功能以及测试
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <util.h>

uint log_2(uint x) {
  	uint pow;
  	int log=0;
  	for(pow=1;pow!=x&&pow<9999999;pow*=2){
  		log++;
  	}
  	if(pow>=9999999)//表示该幂不存在
  		return -1;
  	return log;
}

uint pow_2(uint pow) {
  	return 1 << pow;
}

void util_test() {//测试小功能是否正常运行，不正常就assert结束
  	printf("测试 log_2\n");
  	assert(log_2(1UL) == 0);
  	assert(log_2(2UL) == 1);
  	assert(log_2(4UL) == 2);
  	assert(log_2(1048576UL) == 20);//顺便测试下大数
  	assert(log_2(2097152UL) == 21);
  	assert(log_2(3) == -1);
  	assert(log_2(12) == -1);
  	printf("测试通过\n");

  	printf("测试 pow_2\n");
  	assert(pow_2(0) == 1UL);
  	assert(pow_2(1) == 2UL);
  	assert(pow_2(2) == 4UL);
  	assert(pow_2(3) == 8UL);
  	assert(pow_2(20) == 1048576UL);
  	assert(pow_2(21) == 2097152UL);
  	printf("测试通过\n");
}
