#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kernel.h> /* for sprintf() */ 
#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h> /* for get_user and put_user */ 
#include <asm/errno.h> 
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/string.h> 
#include <linux/sysfs.h> 

static int device_open(struct inode *, struct file *); 
static int device_release(struct inode *, struct file *); 
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t device_write(struct file *, const char __user *, size_t, 
                            loff_t *);
static long int device_ioctl(struct file *,unsigned int ioctl_num , unsigned long ioctl_param);
// static ssize_t bytes_registered_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) ;

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUFFER 64 // BUFFER SIZE
#define IOCTL_CHANGEKEY _IOW('n' ,1,int) //ioctl cmd for case we get new key

static int major; // major  number assigned to our device
volatile static int is_open=0; // if we opened device
static int write_offset=0; //where to write on the next write call


static int key_pos=0; //for our encryption procedure
static char message[BUFFER]; //where our data is located
static unsigned int buffer_size=0; // length of message how many bytes writed in BUFFER

static int key=0;// key to encrypt data message[0-4] xor key because key is 4 bytes
module_param(key, int , 0000); //decleration so we can assign on insmod chardev.ko key=(some value)
MODULE_PARM_DESC(key, "An integer");

static struct class *cls;

// static int bytes_registered=0;
// static struct kobject *mymodule; 
// static struct kobj_attribute bytes_registered_attr = 
//                     __ATTR(bytes_registered, 0660, bytes_registered_show, NULL); 


//file operations struct we assign each method
static struct file_operations chardev_fops = { 
    .owner = THIS_MODULE,
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release, 
    .unlocked_ioctl = device_ioctl,

}; 
//initilaize
static int __init chardev_init(void){
    printk(KERN_INFO "(chardev init)\n");
    memset(message,0,BUFFER);

    major= register_chrdev(0,DEVICE_NAME,&chardev_fops);
    if(major<0){
        pr_alert("Register major failed\n");
        return major;
    }

    pr_info("registered major %d number\n",major);

    buffer_size=strlen(message);

    cls = class_create(THIS_MODULE, DEVICE_NAME); 
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 
    printk(KERN_INFO "key = %d\n",key);
    pr_info("device created on /dev/%s \n",DEVICE_NAME);
    return SUCCESS;
}



static void __exit chardev_exit(void){
    device_destroy(cls, MKDEV(major, 0)); 

    class_destroy(cls); 

    /* Unregister the device */ 
    unregister_chrdev(major, DEVICE_NAME); 
} 

static int device_open(struct inode * iNode, struct file * myFile){
    if(is_open==1){
        printk(KERN_INFO "already open\n");
        return -EBUSY;
    }
    is_open =1;
    try_module_get(THIS_MODULE);
    return SUCCESS;

}
static int device_release(struct inode * iNode, struct file * myFile){
    if(is_open==0){
        printk(KERN_INFO "ERROR - device  wasnt opened\n");
        return -EBUSY;
    }
    is_open =0;
    module_put(THIS_MODULE);
    return SUCCESS; 
}

//read operation
static ssize_t device_read(struct file * myFile, char __user * buffer, size_t sizeT, loff_t * offset){
    int bytes_read=0;//how many bytes we read
    int read_offset=0;
    unsigned char* bytes_key =(unsigned char*)&key;// casting our key to 4 bytes each byte will be represented as char
    size_t keylen=strlen(bytes_key);
    if(offset ==NULL){
        return -1;
    }

    while((bytes_read<BUFFER) && (*offset < buffer_size)){
        key_pos=read_offset % keylen;//encryption using each one of the bytes 0,1,2,3 .. 4 bytes
        message[read_offset]=message[read_offset]^bytes_key[key_pos];//decrypt
        put_user(message[read_offset],&buffer[*offset]);//printing on user output
        message[read_offset]=message[read_offset]^bytes_key[key_pos];//encrypt
        bytes_read++;
        read_offset++;
        *offset=*offset+1;
        if(read_offset>=BUFFER-1){
            read_offset=0;
        }
    }
    
    printk(KERN_INFO "%s\n",message);

    printk(KERN_INFO "device_read\n");

    return bytes_read;


}
static ssize_t device_write(struct file *myFile, const char __user * buffer, size_t sizeT, 
                            loff_t * offset){
    int bytes_write=0;
    unsigned char* bytes_key =(unsigned char*)&key;
    size_t keylen=strlen(bytes_key);
    if(offset==NULL){
        return -EINVAL;
    }              
    while(bytes_write< sizeT){
        get_user(message[write_offset], &buffer[bytes_write]);
        key_pos=write_offset % keylen;
        message[write_offset]=message[write_offset]^bytes_key[key_pos];//same as read encrypted here
        bytes_write++;
        write_offset++;
        if(write_offset>=BUFFER-1){
            write_offset=0;
        }
    }
    
    buffer_size=strlen(message);

    printk(KERN_INFO "device_write\n");
    printk(KERN_INFO "(%s)",message);
    return bytes_write;
}
static long int device_ioctl(struct file *,unsigned int ioctl_num , unsigned long ioctl_param){
    int newkey=0;//for the new key we get from mioctl
    int keylen=0;//len of key 4bytes
    int idx=0,keypos=0;//same for read
    unsigned char* bytes_key =NULL;
    switch(ioctl_num){
        case IOCTL_CHANGEKEY:
            if(copy_from_user(&newkey, (int __user*)ioctl_param,sizeof(int))){
                return -EFAULT;
            }
            if(key==newkey){
                //same key data is already encrypted
                break;
            }
            printk(KERN_INFO "new key %d\n",newkey);
            //decrypt using old key
            bytes_key = (unsigned char*)&key;
            keylen= strlen(bytes_key);
            for(idx=0; idx<buffer_size;idx++){
                keypos= idx % keylen;
                message[idx]=message[idx]^bytes_key[keypos];
            }
            //encrypt using new key
            key=newkey;
            bytes_key = (unsigned char*)&key;
            keylen= strlen(bytes_key);
            for(idx=0; idx<buffer_size;idx++){
                keypos= idx % keylen;
                message[idx]=message[idx]^bytes_key[keypos];
            }
            break;
        default:
            return -EINVAL;
    }
    return SUCCESS;
}

module_init(chardev_init); 
module_exit(chardev_exit); 

MODULE_LICENSE("GPL");