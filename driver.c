#include "driver.h"
#include "encryptDecrypt.c"
#include "ioct.lh"

//these globals need to be created on module_init and destroyed on module_close.
int global_major;
struct cdev* my_cdev;
struct class* cryptctl_class;
dev_t cryptctl_dev; 

//just make all my variables global lol
char* userMessage;
char* encryptedMessage;


typedef struct pairNode{
    //if these are not freed when module is removed, then we fucked
    struct cdev* enc_cdev;
    struct class* enc_class;
	dev_t enc_dev
} pairNode

pairNode head;
pairNode temp;
int idCounter = 4;

ssize_t encrypt_write(
    struct file* file,
    char __user* user, 
    size_t size,
    loff_t offset    
){

    printk(KERN_ALERT "inside encrypt_write\n");
    return size;
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
    //encrypt("HOUGHTON", userMessage, encryptedMessage);
    decrypt("HOUGHTON",  userMessage, encryptedMessage);
    printk(KERN_ALERT "your word has %s has been deciphered into %s\n", userMessage, encryptedMessage);

    return size;
}


long cryptctl_ioctl(
        struct file *file,
        unsigned int ioctl_cmd,/* The number of the ioctl */
        unsigned long ioctl_param /* The parameter to it */
){

  
    switch(ioctl_cmd){
        case IOCTL_CREATE:
            //create new encode/decode pair and store info somewheres
            ioctl_create();        
            break;
    }

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

void ioctl_create(void){

        //create a fops for encrypt
        struct file_operations* enc_fop = (file_operations*)kmalloc(sizeof(file_operations), GFP_KERNEL);
        enc_fop -> owner = THIS_MODULE;
        enc_fop -> write = encrypt_write;

        char* enc_name = (char*)kmalloc(sizeof(char) * 20, GFP_KERNEL);
        
        //give a name 
        sprintf(enc_name, "fuck%d", idCounter);

        //allocate space for new pairnode
        pairNode* newNode = (pairNode*)kmalloc(sizeof(pairNode), GFP_KERNEL);
        
        //append it to global LL 
        temp -> next = newNode;
        temp = temp -> next;     
   
        //create dev 
        //the minor number of enc device will be 2 * idCounter
        newNode -> enc_dev = MKDEV(MAJOR_NUM, 2 * idCounter); 
        
        if(register_chrdev_region(newNode->enc_dev, 1 , enc_name)){
            printk(KERN_ALERT "failed to register chardev region\n");
        }else{
            printk(KERN_ALERT "registered chardev region!\n");    
        }

        newNode -> enc_cdev = cdev_alloc();
        newNode -> enc_cdev -> ops = enc_fop;
        cdev_init(newNode->enc_cdev, enc_fop);
        if(cdev_add(newNode->enc_cdev, newNode->enc_dev)  < 0){
            printk(KERN_ALERT "failed to add cdev\n");
        }else{
            printk(KERN_ALERT "added cdev\n");
        }

        newNode->enc_class = class_create(THIS_MODULE, enc_name);
        device_create(newNode->enc_class, NULL, newNode->enc_dev, NULL, enc_name);
}


int what(void){
    int i;
    int cryptctl_control_dev; 
    int cdev_ret;
   
    //allocate stuff for global linked list
    head = (pairNode*)kmalloc(sizeof(pairNode), GFP_KERNEL); //dummy node
    temp = head;
 
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

    //major number will be MAJOR_NUM 
    cryptctl_dev = MKDEV(MAJOR_NUM, 0);

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
    //for each thing in the linked list,unregister everythin

    pairNode* t = head -> next;
    while(t != NULL){
        device_destroy(t->enc_class, t->enc_dev);
        class_destroy(t->enc_class);    
        unregister_chrdev_region(t->enc_dev, 1);
        cdev_del(t->enc_cdev);
        printk(KERN_ALERT "deleted sub-device\n");
        t = t -> next;
    } 


    //destroy device file
    device_destroy(cryptctl_class, cryptctl_dev);

    //destroy class
    class_destroy(cryptctl_class);
    //unregister cdev space
    unregister_chrdev_region(MKDEV(MAJOR_NUM,0), 1);

    //unregister cdev numbers(the new way)
    cdev_del(my_cdev);

    printk(KERN_ALERT "exiting cryptctl module\n");
}

module_init(what);
module_exit(exit_module);


