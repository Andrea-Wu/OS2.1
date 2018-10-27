#include "driver.h"
#include "encryptDecrypt.c"

//IOCTL functions
//the 2nd argument should be UNIQUE, 1st arg should be 169 for all 
#define IOCTL_CREATE _IO(169,0)

//these globals need to be created on module_init and destroyed on module_close.
int global_major;
struct cdev* my_cdev;
struct class* cryptctl_class;
dev_t cryptctl_dev; 

//just make all my variables global lol
char* userMessage;
char* encryptedMessage;

ssize_t encrypt_write(
    struct file* file,
    char __user* user, 
    size_t size,
    loff_t offset    
){
    return 0;
}



int cryptctl_open(
    struct inode* inode,
    struct file* file   
){
    printk(KERN_ALERT "opened device\n");
    return 0;
}

int cryptctl_release(
    struct inode* inode,
    struct file* file    
){
    printk(KERN_ALERT "closed device\n");
    return 0;
}
//this should actually be called "encrypt_write" but that's for later
ssize_t cryptctl_write(
    struct file* file,
    const char* buffer, 
    size_t size,
    loff_t* offset          
){
    //write to global buffer
    snprintf(userMessage,size,"%s",buffer);
    encrypt("HOUGHTON", userMessage, encryptedMessage);
    printk(KERN_ALERT "your word has %s has been ciphered into %s\n", userMessage, encryptedMessage);

    return size;
}


long cryptctl_ioctl(
        struct file *file,
        unsigned int ioctl_cmd,/* The number of the ioctl */
        unsigned long ioctl_param /* The parameter to it */
){

   /* 
    switch(ioctl_cmd){
        case IOCTL_CREATE:
            //create new encode/decode pair and store info somewheres
            
            struct file_operations encrypt_fops = {
                owner = THIS_MODULE, //maybe
                .unlocked_ioctl = ,
                .write = 
            } 

    }

*/
    printk(KERN_ALERT "in ioctl\n");
    return 0;
}


//create a file_operations struct for "control device".   
struct file_operations cryptctl_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = &cryptctl_ioctl,
    .open = cryptctl_open,
    //.release = cryptctl_release
    .release = cryptctl_release,
    .write = cryptctl_write
};



int what(void){
    int i;
    int cryptctl_control_dev; 
    int cdev_ret;
    //int dev_creat_ret;
    
    //allocate space for string that user will input & null terminate
    userMessage = (char*)kmalloc(sizeof(char) * 1000,GFP_KERNEL);
    for(i=0;i<1000;i++){
        userMessage[i] = '\0';
    }
    
    //allocate space for encrypted string & null terminate
    encryptedMessage = (char*)kmalloc(sizeof(char) * 1000,GFP_KERNEL);
    for(i=0;i<1000;i++){
        encryptedMessage[i] = '\0';
    }

    //major number will be 169 
    cryptctl_dev = MKDEV(169, 0);

   //register a minor number for the cotrol device
    cryptctl_control_dev = register_chrdev_region(cryptctl_dev, 1, "control_device");
  
   //register char driver (the new way) 
    my_cdev = cdev_alloc();
    my_cdev -> ops = &cryptctl_fops;
    cdev_init(my_cdev, &cryptctl_fops);
    cdev_ret = cdev_add(my_cdev, cryptctl_dev, 1);


    printk(KERN_ALERT "this number is 0 if the module is inserted: %d", cdev_ret);

    //create a device file in /dev/cryptctl
    cryptctl_class = class_create(THIS_MODULE, "cryptctl_class");
    device_create(cryptctl_class, NULL, cryptctl_dev, NULL, "cryptctl");
    
    return 0;

}

void exit_module(void){
    //unregister cdev (the old way)
    //unregister_chrdev(global_major, "cryptctl");

    //destroy device file
    device_destroy(cryptctl_class, cryptctl_dev);

    //destroy class
    class_destroy(cryptctl_class);
    //unregister cdev space
    unregister_chrdev_region(MKDEV(169,0), 1);

    //unregister cdev numbers(the new way)
    cdev_del(my_cdev);

    printk(KERN_ALERT "exiting cryptctl module\n");
}

module_init(what);
module_exit(exit_module);


