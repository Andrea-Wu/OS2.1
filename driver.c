#include "driver.h"
#include "encryptDecrypt.c"
#include "ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

//func prototypes
void ioctl_create(int id);
void ioctl_delete(int id);

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
    int id;

    struct cdev* enc_cdev;
    struct class* enc_class;
	dev_t enc_dev;

    char* userStringEncrypt;
        
    struct pairNode* next;
} pairNode;

pairNode* head;
pairNode* temp;

ssize_t encrypt_write(
    struct file* file,
    const char* user, 
    size_t size,
    loff_t* offset    
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

    int id;
  
    switch(ioctl_cmd){
        case IOCTL_CREATE:
            //unpack the ioctl param  
            id = (int)ioctl_param;
            printk(KERN_ALERT "ioctl create recieved id %d\n", id);

            //create new encode/decode pair and store info somewheres
            ioctl_create(id);        
            break;
        case IOCTL_DESTROY:
            //unpack ioctl param
            id = (int)ioctl_param;
            printk(KERN_ALERT "ioctl delete recieved id is %d\n", id);
            ioctl_delete(id);
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

void ioctl_create(int id){
        char* enc_name;
        pairNode* newNode;
        pairNode* itr;
        //create a fops for encrypt
        struct file_operations* enc_fop;

        //make sure the key doesn't exist already by searching global LL (starting from head->next) 
                                                                       //b/c head is dummy node
        if(head == NULL){
            printk(KERN_ALERT "idk why head would be null\n");
            return;
        }

        //0 is the minor number of the cryptctl device
        if(id == 0){
            printk(KERN_ALERT "can't register ID of 0, sorry\n");
            return;
        }
        
        itr = head -> next;
        while(itr != NULL){
            if(itr -> id == id){
                printk(KERN_ALERT "key with id %d already exists\n", id);
                return;
            }
            itr = itr -> next;
        }


        enc_fop = (struct file_operations*)kmalloc(sizeof(struct file_operations), GFP_KERNEL);
        enc_fop -> owner = THIS_MODULE;
        enc_fop -> write = cryptctl_write;

        
        enc_name = (char*)kmalloc(sizeof(char) * 20, GFP_KERNEL);
        
        //give a name 
        sprintf(enc_name, "fuck%d", id);

        //allocate space for new pairnode
        newNode = (pairNode*)kmalloc(sizeof(pairNode), GFP_KERNEL);
       
        //init some struct values
        newNode -> id = id;
        newNode -> next = NULL;        
 
        //append it to global LL 
        temp -> next = newNode;
        temp = temp -> next;     
   
        //create dev 
        //the minor number of enc device will be 2 * id
        newNode -> enc_dev = MKDEV(MAJOR_NUM, 2 * id); 
  
        //register chraracter device region      
        if(register_chrdev_region(newNode->enc_dev, 1 , enc_name)){
            printk(KERN_ALERT "failed to register chardev region\n");
        }else{
            printk(KERN_ALERT "registered chardev region!\n");    
        }
        
        
        //allocating cdev space
        newNode -> enc_cdev = cdev_alloc();
        newNode -> enc_cdev -> ops = enc_fop;
        cdev_init(newNode->enc_cdev, enc_fop);
        if(cdev_add(newNode->enc_cdev, newNode->enc_dev, 1)  < 0){
            printk(KERN_ALERT "failed to add cdev\n");
        }else{
            printk(KERN_ALERT "added cdev\n");
        }

        //creating class and device
        newNode->enc_class = class_create(THIS_MODULE, enc_name);
        device_create(newNode->enc_class, NULL, newNode->enc_dev, NULL, enc_name);
        
}

void ioctl_delete(int id){
    pairNode* itr_tmp;
    pairNode* itr;
    if(head == NULL){
        printk(KERN_ALERT "idk why head would be null\n");
        return;
    }

    itr_tmp = head;
    itr = head -> next;
    while(itr != NULL){
        printk(KERN_ALERT "at id %d\n", itr->id);
        if(itr -> id == id){
            //remove itr from list
            itr_tmp -> next = itr_tmp -> next -> next;
            
            //removing device bookkeeping                        
            device_destroy(itr->enc_class, itr->enc_dev);
            class_destroy(itr->enc_class);    
            unregister_chrdev_region(itr->enc_dev, 1);
            cdev_del(itr->enc_cdev);

            //TODO: free node

            printk(KERN_ALERT "deleted sub-device with id %d\n", id);
            break;
     
        }
        itr_tmp = itr;
        itr = itr -> next;
    }
    
    if(itr == NULL){
        printk(KERN_ALERT "device with id %d could not be deleted bc it doesn't exist\n", id);
    }


}


int what(void){
    int i;
    int cryptctl_control_dev; 
    int cdev_ret;
   
    //allocate stuff for global linked list
    head = (pairNode*)kmalloc(sizeof(pairNode), GFP_KERNEL); //dummy node
    head -> next = NULL;
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
        printk("LOOPZ\n");
        device_destroy(t->enc_class, t->enc_dev);
        class_destroy(t->enc_class);    
        unregister_chrdev_region(t->enc_dev, 1);
        cdev_del(t->enc_cdev);
        printk(KERN_ALERT "deleted sub-device\n");
        t = t -> next;
    } 

    printk(KERN_ALERT "fuck1");
    //destroy device file
    device_destroy(cryptctl_class, cryptctl_dev);
    printk(KERN_ALERT "fuck2");

    //destroy class
    class_destroy(cryptctl_class);

    printk(KERN_ALERT "fuck3");
    //unregister cdev space
    unregister_chrdev_region(MKDEV(MAJOR_NUM,0), 1);

    printk(KERN_ALERT "fuck4");
    //unregister cdev numbers(the new way)
    cdev_del(my_cdev);
    printk(KERN_ALERT "fuck5");

    printk(KERN_ALERT "exiting cryptctl module\n");
}

module_init(what);
module_exit(exit_module);


