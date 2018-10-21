#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/device.h>



MODULE_LICENSE("Dual BSD/GPL");

//these globals need to be created on module_init and destroyed on module_close.
int global_major;
struct cdev* my_cdev;
struct class* cryptctl_class;
dev_t cryptctl_dev; 

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


long cryptctl_ioctl(
        struct file *file,
        unsigned int ioctl_num,/* The number of the ioctl */
        unsigned long ioctl_param /* The parameter to it */
){
    printk(KERN_ALERT "in ioctl\n");
    return 0;
}



int what(void){
   
    int cryptctl_control_dev; 
    int cdev_ret;
    int mknod_ret;
    //create a file_operations struct for "control device".   
    struct file_operations cryptctl_fops = {
        .owner = THIS_MODULE,
        .unlocked_ioctl = &cryptctl_ioctl,
        .open = cryptctl_open,
        .release = cryptctl_release
    };

    //register this this file as device driver (the old way)
    //int major = register_chrdev(70, "cryptctl", &cryptctl_fops);
    //turn "control device" information into a dev_t object

 
    //major number will be 169 
    cryptctl_dev = MKDEV(169, 0);

   //register a minor number for the cotrol device
    cryptctl_control_dev = register_chrdev_region(cryptctl_dev, 1, "control_device");
  
   //register char driver (the new way) 
    my_cdev = cdev_alloc();
    my_cdev -> ops = &cryptctl_fops;
    cdev_init(my_cdev, &cryptctl_fops);
    cdev_ret = cdev_add(my_cdev, cryptctl_control_dev, 1);

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

    //unregister cdev space
    unregister_chrdev_region(MKDEV(169,0), 1);

    //unregister cdev numbers(the new way)
    cdev_del(my_cdev);

    printk(KERN_ALERT "exiting cryptctl module\n");
}

module_init(what);
module_exit(exit_module);


