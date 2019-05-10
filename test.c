#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<linux/ioctl.h>

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

void error_close(int fd,unsigned int type);
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
    fd = open("/dev/my_dev",O_RDWR);
    if(fd<0)
    {
        perror("open my_dev fair!\n");
        exit(-1);
    }
    printf("open my_dev success!\n");
    ret = write(fd,buf,strlen(buf));
    if(ret==-1)
    {
        error_close(fd,WRITE);
    }
    ret = read(fd,temp,sizeof(temp));
    if(ret == -1)
    {
        error_close(fd,READ);               
    }
    printf("read len=%d,temp=%s!\n",ret,temp);

    ret = ioctl(fd,cmd,0);
    if(ret == -1)
    {
        error_close(fd,IOCTL);
    }
    close(fd);
    return 0;
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


