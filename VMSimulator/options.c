/*
	处理命令行选项，初始化全局选项.
*/

//#include <config.h>

#include <unistd.h>//用于getopt
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <vmsim.h>
#include <stats.h>
#include <options.h>
#include <fault.h>
#include <util.h>

#define MIN_PHYS_PAGES 3//定义最小物理页面数
#define MIN_PAGESIZE 16//最小页面大小，power of 2

/* 全局结构体。Process_options将设置它的值 */
opts_t opts;
stats_t *stats;

static const char *shortopts = "hvtap:s:l:o:";//定义哪些字符有效

#define GETOPT(argc, argv) getopt(argc, argv, shortopts)

static void options_handle_algorithm(const char *alg_name);
static long options_atoi(const char *arg);
static void options_print_help();
static void algorithm_help();
static void addr_create();

/* 处理argc/argv数组，更新全局选项结构'opts'。 */
/*char *optarg：如果有参数，则包含当前选项参数字符串
int optind：argv的当前索引值。当getopt函数在while循环中使用时，剩下的字符串为操作数，下标从optind到argc-1。
int opterr：这个变量非零时，getopt()函数为“无效选项”和“缺少参数选项，并输出其错误信息。
int optopt：当发现无效选项字符之时，getopt()函数或返回 \’ ? \’ 字符，或返回字符 \’ : \’ ，并且optopt包含了所发现的无效选项字符。
————————————————*/

void options_process(int argc, char **argv) {
  	int opt;
  	/* 在此函数中处理的选项: */
  	int help = FALSE;

  	opts.addr_create = FALSE;
  	opts.output_file = NULL;
  	opts.input_file = NULL;
  	opts.verbose = FALSE;
  	opts.test = FALSE;
  	opts.pagesize = 1024;
  	opts.phys_pages = 128;
  	opts.limit = 0;
  
  	while (1) {
    		opt = GETOPT(argc, argv);//从终端获取输入，由optarg接收选项参数
    		if (opt == -1)
      			break;
    		switch (opt) {
    			case 'h':
      				help = TRUE;
      				break;
      			case 'a':
      				opts.addr_create = TRUE;
      				break;
    			case 'v':
      				opts.verbose = TRUE;
      				break;
    			case 'o':
      				opts.output_file = optarg;//输出文件名
      				break;
    			case 'l':
      				opts.limit = options_atoi(optarg);
      				break;
   			case 't':
      				opts.test = TRUE;
      				break;
    			case 'p':
      				opts.phys_pages = options_atoi(optarg);
      				break;
    			case 's':
      				opts.pagesize = options_atoi(optarg);
      				break;
    			case '?':
      				// 无法识别的选项，打印使用
      				help = TRUE;
      				break;
    			default:
      				printf("getopt returned '%c' - unhandled option\n", opt);
      				abort();//异常退出
    		}
  	}
  	
  	if(opts.addr_create){
		addr_create();
		exit(0);
	}//创建一个地址流
  
  	if (help) {
    		options_print_help();
    		exit(0);
  	}

	
	
  	if (opts.limit < 0) {
    		fprintf(stderr, "vmsim: limit must be > 0\n");
    		exit(1);
  	}

  	if (opts.phys_pages < MIN_PHYS_PAGES) {
    		fprintf(stderr, "vmsim: must have at least %d pages\n", MIN_PHYS_PAGES);
    		exit(1);
  	}

  	if (opts.pagesize < MIN_PAGESIZE) {
    		fprintf(stderr, "vmsim: pagesize must be at least %d bytes\n", MIN_PAGESIZE);
    		exit(1);
  	}
  	if (log_2(opts.pagesize) == -1) {
    		fprintf(stderr, "vmsim: pagesize must be a power of 2\n");
    		exit(1);
  	}

/*变量 optind 是要处理的 argv[] 向量的下一个元素的索引。 它应该由系统初始化为 1，并且 getopt() 应该在它完成 argv[] 的每个元素时更新它。 

argc用来统计你运行程序时送给main函数的命令行参数的个数。
* argv[ ]: 字符串数组，用来存放指向你的字符串参数的指针数组，每一个元素指向一个参数.
argv[0] 指向程序运行的全路径名
argv[1] 指向在DOS命令行中执行程序名后的第一个字符串
argv[2] 指向执行程序名后的第二个字符串
————————————————*/

  	if (optind >= argc) {
    		fprintf(stderr, "vmsim: 必须是指定的算法\n");
    		exit(1);
  	}
  	options_handle_algorithm(argv[optind]);//argv[optind]仍是一个指针，指向算法名称字符串首地址，
  
 	 if (optind+1 < argc) {
    		opts.input_file = argv[optind+1];
  	}

  	if (opts.input_file == NULL || strcmp("-", opts.input_file) == 0) {
    		opts.input_file = NULL;
    		if (opts.verbose) {
      			printf("vmsim: reading from stdin\n");
    		}
  	} 
  	else if (opts.verbose) {
    		printf("vmsim: reading from %s\n", opts.input_file);
  	}

  	if (opts.verbose && opts.output_file) {
    		printf("vmsim: sending output to %s\n", opts.output_file);
  	}
}

void addr_create(){
	int n;
	int i;
	int num;
	stats_init();
	FILE *o = stats->output;
	FILE *d = fopen("random_test.txt","w");//用于清除先前内容,指定文件
	printf("请输入生成随机地址数量:\n");
	scanf("%d",&n);
	
	srand((unsigned)time(NULL));
	
	for(i = 1; i <= n*17; i++){
		if(i%17==2 || i%17==5)
			fprintf(o, "%c",',');//逗号
		if(i%17==3 || i%17==6)
			fprintf(o, " ");//空格
		if(i%17==12 || i%17==7 || i%17==11 || i%17==9 || i%17==10)
			fprintf(o, "%d",0);//地址前四位不妨定为0
		if(i%17==1)
			fprintf(o ,"%d",1);//不妨让进程pid均为1，也可通过随机数调整
		if(i%17==4){
			num=random()%2;
			if(num==0)
				fprintf(o ,"%c",'R');//读，也可调整为其他
			if(num==1)
				fprintf(o ,"%c",'W');//写
		}			
		if(i%17==8)
			fprintf(o ,"%c",'x');
		
		if(i%17==13){
			//num = random()%16;//生成一个0～15的数
			//fprintf(o ,"%x",num);//以16进制输出
			fprintf(o ,"%x",14);
		}
		if(i%17==14){
			
			
			while(1){
				num = random()%16;//生成一个13～15的数
				//printf("num = %d\n",num);
				if(num >= 13){break;}
					
			}
			fprintf(o ,"%x",num);//以16进制输出
			//fprintf(o ,"%x",15);
		}	
		if(i%17==15){
			num = random()%16;//生成一个0～15的数
			fprintf(o ,"%x",num);//以16进制输出
			//fprintf(o ,"%x",3);
		}
		if(i%17==16){
			num = random()%16;//生成一个0～15的数
			fprintf(o ,"%x",num);//以16进制输出
		}
		if(i%17==0)
			fprintf(o ,"\n");//最后一位输出换行符	
	}
	fclose(d);
}
long options_atoi(const char *arg) {//提取长整形
  	char *end;
  	long ret;
  	ret = strtol(arg, &end, 10);//提取出字符串end中的长整形部分
  	if (*end != '\0') {//非空
    		fprintf(stderr, "vmsim: invalid integer argument: %s\n", arg);
    		exit(1);
  	}
  	return ret;
}

void options_handle_algorithm(const char *alg_name) {//寻找匹配的算法名
  	fault_handler_info_t *alg;
  	for (alg = fault_handlers; alg->name != NULL; alg++) {
    		if (strcmp(alg->name, alg_name) == 0) {//指针一直后移，直到找到一个相同名称的算法名
      			break;
    		}
  	}
  	if (alg->name == NULL) {//没找到
    		fprintf(stderr, "vmsim: 没有算法 '%s' 可用\n", alg_name);
    		exit(1);
  	} 
  	else if (opts.verbose) {//要求详细输出
    		printf("vmsim: 使用 '%s'替换算法\n", alg_name);
  	}
  	opts.fault_handler = alg;
}


void options_print_help() {
  	printf("程序说明：\n\n");
  	printf("使用方法: vmsim [OPTIONS] ALGORITHM [TRACEFILE]\n\n");
  	printf("进程TRACEFILE，模拟虚拟机系统。报告分页行为的统计信息。\n");
  	printf("如果TRACEFILE没有指定文件或者是'-'，输入将从stdin获取。\n");
  	printf("\n");
  	printf("ALGORITHM为错误处理算法:\n");
  	algorithm_help();
  	printf("\n");
  	printf("OPTIONS:\n");
  	printf("-h               打印此消息并退出.\n");
  	printf("-v               详细的输出。包括进程输出.\n");
  	printf("-a               随机生成地址流，后跟-o FILE指明输出目的文件\n");
  	printf("-t               运行的自我测试.\n");
  	printf("-o FILE          将统计输出附加到给定的文件中.\n");
  	printf("-l REFS          在第一次参考后停止.\n");
  	printf("-p PAGES         模拟页面物理页面.\n");
  	printf("                 最小值 %d.\n", MIN_PHYS_PAGES);
  	printf("-s SIZE          拟size字节的页面大小.\n");  
  	printf("                 大小必须是2的幂.\n");
  
}

void algorithm_help() {
  	fault_handler_info_t *alg;
  	printf("   ");
  	for(alg = fault_handlers; alg->name!=NULL; alg++){
  	  	printf("%-10s", alg->name);
  	}
  	printf("\n");
}//依次输出各个算法名称
