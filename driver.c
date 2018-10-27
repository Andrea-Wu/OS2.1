#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/splice.h>
#include <linux/slab.h>
MODULE_LICENSE("Dual BSD/GPL");

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

void cipher(char* key){
    //TODO: for now the global userMessage == str because I'm not 100% sure 
    //  how this will be implemented- will writing 2 strings consecutively to file concat them,
    //  or will second string overwrite the first?
    char* str = userMessage;
    int key_len = strlen(key);
    int enc_len = strlen(encryptedMessage); //the length of the old encryptedMessage that we will clear
    int str_len = strlen(str);
    int key_counter = 0; //this is used to cycle thru characters in key string
    char row; //this is a row of the "table" for ciphering
    char col; //this is a column of the "table" for ciphering
    //(note that we're not creating an actual table)
    
    int new_enc_len = 0; //this is for keeping track of the new encryptedMessage length
    char str_i;

    //clear encryptedMessage
    int i;
    for(i=0; i<enc_len; i++){
        encryptedMessage[i] = '\0';
    }

    //make sure all letters in key are capitalized or else this algorithm will break
    for(i=0; i<key_len;i++){
         if(key[i] >= 97 && key[i] <= 122){
            key[i] = key[i] - 22;
        }
    }
    
    //the cipher algorithm

    for(i=0;i<str_len; i++){
        //if character is non-alphabetic, just ignore it
        //this is essentially isalpha(), which isn't available in kernel
        str_i = str[i];
        if(!(str_i >= 97 && str_i <= 122) && !(str_i >= 65 && str_i <=90)){
            continue;
        }

        //if letter is not capitalized, make it capitalized
        if(str_i >= 97 && str_i <= 122){
            str[i] = str_i - 32;
        }
        
        //value @ key[key_counter] = row -> row #
        //value @ str[i] = col -> col#
        
        //mapping characters to numbers 
        //for example, A:0, B:1, ... Z:25
        row = key[key_counter] - 65; 
        col = str[i] -65;

        //the value of the char in the return string is (( row + col) % 26) + 65
        encryptedMessage[new_enc_len] = (char)(((row+col)%26) + 65);

        new_enc_len++;
        key_counter++;
        //if counter exceeds index of key, reset to 0
        if(key_counter == key_len){
            key_counter = 0;
        }
    }

    //append null terminator to encrypted string
    encryptedMessage[new_enc_len] = '\0';    
}

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

ssize_t cryptctl_write(
    struct file* file,
    const char* buffer, 
    size_t size,
    loff_t* offset          
){

    
    //write to global buffer
    snprintf(userMessage,size,"%s",buffer);
    cipher("HOUGHTON");
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


