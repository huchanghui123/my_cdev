//#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>
#include<linux/string.h>
#include <asm/msr.h>


/*定义设备类型*/
#define IOC_MAGIC 'c'
/*读寄存器*/
#define IOC_COMMAND1 _IOW(IOC_MAGIC,1,int)
#define IOC_COMMAND2 _IOW(IOC_MAGIC,2,int)

//创建一个字符设备
struct char_dev
{
    struct cdev c_dev;
    dev_t dev_no;
    char buf[1024];
};
struct class *cls;
struct device *my_device;
static u8 temp[64];

int my_open(struct inode *inode, struct file *file)
{
    printk("my cdev open!\n");
    return 0;
}

int my_close(struct inode *inode, struct file *file)
{
    printk("my cdev close!\n");
    return 0;
}

ssize_t my_read(struct file *file,char __user *buf,size_t len,loff_t *pos)
{
    if(len > 64)
        len = 64;
    char ker_buf[200] = "Kernel say:";
    //printk("ker_buf :%s temp:%s\n", ker_buf, temp);
    strcat(ker_buf, temp);
    if(copy_to_user(buf,ker_buf,strlen(ker_buf)))
        return -EFAULT;
    return len;
}

ssize_t my_write(struct file *file,const char __user *buf,size_t len,loff_t *pos)
{
    if(len>64)
        len = 64;
    if(copy_from_user(temp,buf,len))
        return -EFAULT;
    printk("my cdev write:%s\n",temp);
    return len;
}

long my_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    printk("ioctl cmd:%d \n",cmd);
    switch(cmd)
    {
        case IOC_COMMAND1:
            printk("my ioctl command 1 successfully!\n");

            u64 __val = __rdmsr(0x1A2);
            printk("__val is %016llx\n",__val);
            
            u64 __val2 = __rdmsr(0x19C);
            printk("__val2 is %016llx\n",__val2);

            u8 tjunction,delta,coretemp;
            tjunction = (__val>>16)&0x7f;
            delta = (__val2>>16)&0x7f;
            coretemp = tjunction - delta;

            printk("tjunction is %.1d°C ;delta is %.1d°C;core temp is %.1d°C\n", tjunction, delta, coretemp);

            char buf_temp[30] = "CPU温度:";
            char core_temp[4];
            sprintf(core_temp, "%d", coretemp);
            strcat(buf_temp,core_temp);
            copy_to_user((char __user *)arg, buf_temp, strlen(buf_temp));

			u32 lo,hi;
			asm volatile(
				"rdmsr"
				: "=a" (lo), "=d" (hi)
				: "c" (0x1A2)
			);
			u64 value = ((u64)hi<<32)|lo;
			printk("0x1a2 value :0x%016llx \n", value);

            break;
        case IOC_COMMAND2:
            printk("my ioctl command 2 successfully!\n");
            break;
        default:
            printk("my ioctl error!\n");
            return -EFAULT;
    }
    return 0;
}

struct file_operations my_ops = {
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_ioctl,
    .release = my_close,
};

struct char_dev *my_dev;
//dev_t dev_no;
static int __init cdev_test_init(void)
{  
    int ret;
    printk("HELLO KERNEL FOR MY CDEV!\n");
    //创建设备号->主设备号，次设备号
    //dev_no = MKDEV(222,2);
    //注册设备号
    //ret = register_chrdev_region(dev_no,1,"my_dev");
    //1.给字符设备分配内存空间
    my_dev = kmalloc(sizeof(*my_dev),GFP_KERNEL);
    if(!my_dev)
    {
        ret = -ENOMEM;
        goto malloc_dev_fair;
    }
    //2.自动申请设备号并注册字符设备
    ret = alloc_chrdev_region(&my_dev->dev_no,1,1,"my_dev");
    if(ret < 0)
    {
        goto alloc_chrdev_fair;
    }
    //3.初始化字符设备
    cdev_init(&my_dev->c_dev, &my_ops);
    //4.添加一个字符设备
    ret = cdev_add(&my_dev->c_dev, my_dev->dev_no, 1);
    if(ret < 0)
    {
        goto cdev_add_fair;
    }
    //5.为该设备创建一个class
    //这个类存放于sysfs下面，调用device_create函数时会在/dev目录创建相应的设备节点
    cls = class_create(THIS_MODULE, "myclass");//sys/devices/virtual/myclass/my_dev
    if(IS_ERR(cls))
    {
        unregister_chrdev_region(my_dev->dev_no,1);
        return -EBUSY;
    }
    //6.创建对应的设备节点
    //加载模块时，用户空间的udev会自动响应该函数，去/sysfs下寻找对应的类创建设备节点
    my_device = device_create(cls,NULL,my_dev->dev_no,NULL,"my_dev");//mknod /dev/my_dev
    if(IS_ERR(my_device))
    {
        class_destroy(cls);
        unregister_chrdev_region(my_dev->dev_no,1);
        return -EBUSY;
    }
    return 0;
cdev_add_fair:
    return ret;
malloc_dev_fair:
    return ret;
alloc_chrdev_fair:
    return ret;

}

static void __exit cdev_test_exit(void)
{
    //删除设备
    //cdev_del(&my_dev->c_dev);
    device_destroy(cls, my_dev->dev_no);
    class_destroy(cls);
    //注销驱动——>后面写1表示从dev_on开始连续一个
    unregister_chrdev_region(my_dev->dev_no,1);
    kfree(my_dev);
    printk("GOODBYE, MY CDEV!\n");
}

module_init(cdev_test_init);
module_exit(cdev_test_exit);
MODULE_LICENSE("GPL");
