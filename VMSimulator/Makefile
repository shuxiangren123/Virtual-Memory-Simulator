
#reference:https://blog.csdn.net/songweiren1/article/details/118569899

#依赖关系

vmsim: vmsim.o fault.o options.o pagetable.o physmem.o stats.o util.o	
vmsim.o: vmsim.c  
	gcc -DUNIX -g -Wall -I. -c vmsim.c
fault.o: fault.c
	gcc -DUNIX -g -Wall -I. -c fault.c 
options.o: options.c
	gcc -DUNIX -g -Wall -I. -c options.c
pagetable.o: pagetable.c  
	gcc -DUNIX -g -Wall -I. -c pagetable.c
physmem.o: physmem.c  
	gcc -DUNIX -g -Wall -I. -c physmem.c
stats.o: stats.c  
	gcc -DUNIX -g -Wall -I. -c stats.c
util.o: util.c  
	gcc -DUNIX -g -Wall -I. -c util.c

.PHONY: depend clean
clean:
	-rm -rf *.o
run:
	#@./vmsim

