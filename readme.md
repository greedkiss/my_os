# 日程记录
### 3.1 
main.c，memory.c等文件，第一步内存初始化操作，并进行单元测试，copy_page汇编转C
### 3.2
memory.h中定义read_swap_page 从交换分区读内存页接口未完成，也不可用之前的文件系统接口，需要将请求插入请求队列排序  
出现了一个问题，物理内存是malloc申请得到但是需要define　LOW_MEM，结果会报错需要用常量来定义LOW_MEM这也就导致只能在每个函数体中重复定义该变量，也就是在main函数执行之前编译器会先定义全局变量，所以不可用运行时定义的局部变量定义全局变量  
所有的线性地址必须得减去RAM（起始地址）,现在的64位编译器unsigned long为８字节
### 3.3
LOW_MEM还是应该定义为0x100000，所有的addr线性地址减去RAM
### 3.4
引用了TASK_SIZE和current 当前进程块指针，还有读交换分区未完成  
当前struct task_struct（进程数据结构）还未定义，当前m_inode未定义（内存i节点)  
任务数组未定义, FIRST_TASK和LASK_TASK未定义，library_offset未定义，进程印象偏移值之上为库文件，之下为可执行文件  
bmap() bread_page()未定义，他们是从磁盘中读取文件块  
缺页处理的三种情况１.从交换分区读２.和其他进程共享3.从磁盘读(可能是库文件也可能是可执行文件)  
show_mem()可以用于键盘外设中断调用,<font color=red>pg_dir[]未定义</font>，为从０地址开始的目录表
### 3.5
0.12中的系统调用都是用汇编实现，可以直接操作寄存器，包括可以使用call等跳转语句修改cs ip实现跳转。比如时钟中断，１０ms一个滴答，如何实现记录原进程切换时的cs ip值是一个问题，如果时间到了我从外面休眠该进程，进程这块涉及到底层硬件的确是个问题。那就还是调用底层的wait等只不过在外面封装下，来模拟保存cs ip然后系统调用当前的底层接口。
### 3.7
blk_size[MAJOR(SWAP_DEV)][MINOR(SWAP_DEV)]取数据块总数（外存以1KB为一块)  
用一个文件代替交换分区，同文件系统中硬盘初始化操作(内存管理除了一个系统调用和交换分区初始化全部完成)
### sched.c
增加进程调试信息，show_state,可用作键盘驱动  
有define LATCH 初始化8253芯片,用c语言时间类库来代替8253  
定义current 即当前运行任务指针和任务数组task  
定义了schedul调度函数， 引用<font color=red>switch_to未定义</font>  
系统调用**do_timer() sys_alarm()** 和 初始化程序**sched_init()**