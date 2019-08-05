#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include<pthread.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<linux/ioctl.h>
#include <errno.h>
//#include <asm/msr.h>

/*定义设备类型*/
#define IOC_MAGIC 'c'
/*读寄存器*/
#define IOC_COMMAND1 _IOW(IOC_MAGIC,1,int)
#define IOC_COMMAND2 _IOW(IOC_MAGIC,2,int)

#define WRITE 1
#define READ 2
#define IOCTL 3

char buf[]="我是谁？我来自哪里？人生的意义是什么？";
char temp[64]={0};
char *command[] = {"cmd1","cmd2"};
char core_temp[40];

int open_mydec(int fd, char *path);
void read_mydec(int fd,char *buf);
void write_mydec(int fd,char *buf);
void error_close(int fd,unsigned int type);
void icotl_mydec(int fd,int cmd,char *temp, int core);

void cpu_run(int n) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(n, &mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    printf("cpu: %d---%d ",n, sched_getcpu());
}

int main(int argc, char *argv[])
{
    int cmd;
    printf("Total %d arguments\n",argc);
    if(argc>1)
    {
        if(strcmp(argv[1],command[0])==0)
            cmd = IOC_COMMAND1;
        else if(strcmp(argv[1],command[1])==0)
            cmd = IOC_COMMAND2;
    }
    int fd,ret;

    fd = open_mydec(fd, "/dev/my_dev");

    write_mydec(fd, buf);

    read_mydec(fd,temp);

    for(int i=0; i<100; i++)
    {
    	int pid = fork();
    	if(pid<0)
    	{
    		printf("error in fork!\n");
    	}
    	else if(pid == 0)
    	{
    		cpu_run(1);
    		icotl_mydec(fd, cmd, core_temp, 1);
    		exit(0);
    	}else{
    		cpu_run(0);
   			icotl_mydec(fd, cmd, core_temp, 0);
   			sleep(1);
   			wait();
    	}
    }

    close(fd);

    return 0;
}

int open_mydec(int fd, char *path)
{
	fd = open("/dev/my_dev",O_RDWR);
    if(fd<0)
    {
        perror("open my_dev fair!\n");
        exit(-1);
    }
    return fd;
    printf("open my_dev success!\n");
}

void write_mydec(int fd,char *buf)
{
	int ret = write(fd,buf,strlen(buf));
    if(ret==-1)
    {
        error_close(fd,WRITE);
    }
}

void read_mydec(int fd,char *temp)
{
	int ret = read(fd,temp,sizeof(temp));
    if(ret == -1)
    {
        error_close(fd,READ);               
    }
    printf("read len=%d,temp=%s\n",ret,temp);
}

void icotl_mydec(int fd,int cmd,char *temp, int core)
{
	int ret = ioctl(fd,cmd,core_temp);
    if(ret == -1)
    {
        error_close(fd,IOCTL);
    }
    printf("core %d temp: %s °C\n", core, core_temp);
}

void error_close(int fd,unsigned int type)
{
    switch(type)
    {
        case WRITE:
            perror("write error!\n");
            break;
        case READ:
            perror("read error!\n");
            break;
        case IOCTL:
            perror("ioctl error!\n");
            break;
        default:
            break;
    }
    close(fd);
    exit(-1);

}


