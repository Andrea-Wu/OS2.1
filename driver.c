#include "driver.h"
#include "encryptDecrypt.c"
#include "ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

//func prototypes
int ioctl_create(char* key);
void ioctl_delete(int id);
void ioctl_get_key(int id);
void ioctl_change_key(int id, char* key);

//these globals need to be created on module_init and destroyed on module_close.
int global_major;
struct cdev* my_cdev;
struct class* cryptctl_class;
dev_t cryptctl_dev; 

//just make all my variables global lol
char* userMessage;
char* encryptedMessage;

//global id counter
int idCounter = 1;

//global variable to answer question "which device is being written to?"
int whichDevice = -1;

//TODO: mutex for whichDevice- if multiple users use the command at once. 
//not completely necessary

typedef struct pairNode{
    //if these are not freed when module is removed, then we fucked
    int id;
    char* key;
    struct cdev* enc_cdev;
    struct cdev* dec_cdev;

    struct class* enc_class;
    struct class* dec_class;

	dev_t enc_dev;
    dev_t dec_dev;

    char* encryptRes;
    char* decryptRes;   

    //for kobject shit
    struct kobject* kobj; 
        
    struct pairNode* next;
} pairNode;

pairNode* head;

ssize_t encrypt_write(struct file* file, const char* buffer, size_t size,loff_t* offset){
    //figure out which file is writing:
    struct inode* inode;
    unsigned int minor;
    int id;
    pairNode* itr = head -> next;

    inode = file-> f_path.dentry -> d_inode;
    minor = iminor(inode);
    id = minor/2;

    
    printk(KERN_ALERT "writing %s to device %d\n", buffer,id);
    
    //get the node from global LL
     
    while(itr != NULL){
        if(itr -> id == id){
            snprintf(userMessage,size + 1,"%s",buffer);
            encrypt(itr -> key, userMessage, itr-> encryptRes);
            printk(KERN_ALERT "%s has been ciphered into %s\n", userMessage, itr -> encryptRes);    
            break;
        }
        itr = itr -> next;
    }
    return size;
}

ssize_t encrypt_read(struct file* file, char* buffer, size_t size, loff_t* offset){
    //figure out which file to read from:

    struct inode* inode;
    unsigned int minor;
    int id;
    pairNode* itr = head -> next;

    
    printk(KERN_ALERT "ass1\n");

    inode = file-> f_path.dentry -> d_inode;
    minor = iminor(inode);
    id = minor/2;

    printk(KERN_ALERT "ass2\n");
    
    printk(KERN_ALERT "reading from device %d\n", id);
    
    //get the node from global LL
     
    while(itr != NULL){
        if(itr -> id == id){
            sprintf(buffer, "%s", itr -> encryptRes);   
            return strlen(itr -> encryptRes);
        }
        itr = itr -> next;
    } 

    return 0;

}


ssize_t decrypt_write(struct file* file, const char* buffer, size_t size,loff_t* offset){
    //figure out which file is writing:
    struct inode* inode;
    unsigned int minor;
    int id;

    pairNode* itr = head -> next;

    inode = file-> f_path.dentry -> d_inode;
    minor = iminor(inode);
    id = minor/2;

    
    printk(KERN_ALERT "writing %s to device %d\n", buffer,id);
    
    //get the node from global LL
     
    while(itr != NULL){
        if(itr -> id == id){
            snprintf(userMessage,size + 1,"%s",buffer);
            decrypt(itr -> key, userMessage, itr-> decryptRes);
            printk(KERN_ALERT "%s has been deciphered into %s\n", userMessage, itr -> decryptRes); 
            break;
        }
        itr = itr -> next;
    }
    return size;
}


ssize_t decrypt_read(struct file* file, char* buffer, size_t size, loff_t* offset){
    //figure out which file to read from:

    struct inode* inode;
    unsigned int minor;
    int id;
    pairNode* itr = head -> next;
    

    inode = file-> f_path.dentry -> d_inode;
    minor = iminor(inode);
    id = minor/2;

    printk(KERN_ALERT "ass2\n");
    
    printk(KERN_ALERT "reading from device %d\n", id);
    
    //get the node from global LL
     
    while(itr != NULL){
        if(itr -> id == id){
            sprintf(buffer, "%s", itr -> decryptRes);   
            return strlen(itr -> decryptRes);
        }
        itr = itr -> next;
    } 

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
    char* key;
    changeKeyParam* str_param;
    //char* key;  
    switch(ioctl_cmd){
        case IOCTL_CREATE:
            //unpack the ioctl param  
            key = (char*)ioctl_param;
            //create new encode/decode pair and store info somewheres
            printk(KERN_ALERT "creating sub_device\n");
            ioctl_create(key);        
            break;
        case IOCTL_DESTROY:
            //unpack ioctl param
            id = (int)ioctl_param;
            printk(KERN_ALERT "ioctl delete recieved id is %d\n", id);
            ioctl_delete(id);
            break;
        case IOCTL_GET_KEY:
            id = (int)ioctl_param;
             printk(KERN_ALERT "ioctl get key recieved id is %d\n", id);
            ioctl_get_key(id);
            break;
        
        case IOCTL_CHANGE_KEY:
            str_param = (changeKeyParam*)ioctl_param;
            id = str_param -> id;
            key = str_param-> key;

            printk(KERN_ALERT "changing key of pair %d to %s\n", id,key);
            ioctl_change_key(id, key);
          
    }

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



ssize_t myShow(struct kobject *kobj, struct attribute *attr,char *buffer){
    return 0;
}
 ssize_t myStore(struct kobject *kobj, struct attribute *attr,const char *buffer, size_t size){
   
     return size;
}

int ioctl_create(char* key){
        struct attribute* attr;
        struct attribute* zeroed;

        struct kobj_type* ktype;
        struct kobject* kobj;
        struct sysfs_ops* sysfs_ops;
        struct attribute** attr_arr;

        char* enc_name;
        char* dec_name;        

        char* keyCpy;
        pairNode* newNode;
        
        struct file_operations* enc_fop;
        struct file_operations* dec_fop;
        
        //create a fops for encrypt
        enc_fop = (struct file_operations*)kmalloc(sizeof(struct file_operations), GFP_KERNEL);
        enc_fop -> owner = THIS_MODULE;
        enc_fop -> write = encrypt_write;
        enc_fop -> read = encrypt_read;

        dec_fop = (struct file_operations*)kmalloc(sizeof(struct file_operations), GFP_KERNEL);
        dec_fop -> owner = THIS_MODULE;
        dec_fop -> write = decrypt_write;
        dec_fop -> read = decrypt_read;


        //assign a name for all the things we have to create (use the same name for everythng) 
        enc_name = (char*)kmalloc(sizeof(char) * 20, GFP_KERNEL);
        sprintf(enc_name, "cryptEncrypt%d", idCounter);

        dec_name = (char*)kmalloc(sizeof(char) * 20, GFP_KERNEL);
        sprintf(dec_name, "cryptDecrypt%d", idCounter);

        //allocate space for new pairnode
        newNode = (pairNode*)kmalloc(sizeof(pairNode), GFP_KERNEL);
       
        //init some struct values
        newNode -> id = idCounter;
        newNode -> next = head -> next;        
        keyCpy = (char*)kmalloc(sizeof(char) * 21, GFP_KERNEL);
        sprintf(keyCpy,"%s",key);
        newNode -> key = keyCpy;
        newNode -> encryptRes = (char*)kmalloc(sizeof(char) * 100, GFP_KERNEL);
        newNode -> decryptRes = (char*)kmalloc(sizeof(char) * 100, GFP_KERNEL); 

        //append it to global LL 
        head -> next = newNode;
   
        //create dev 
        //the minor number of encrypt device will be 2 * idCounter
        //the major number will be 2 * idCounter + 1
        newNode -> enc_dev = MKDEV(MAJOR_NUM, 2 * idCounter); 
        newNode -> dec_dev = MKDEV(MAJOR_NUM, 2 * idCounter + 1); 
 
        //register chraracter device region      
        if(register_chrdev_region(newNode->enc_dev, 1 , enc_name)){
            printk(KERN_ALERT "encrypt: failed to register chardev region\n");
        }else{
            printk(KERN_ALERT "encrypt: registered chardev region!\n");    
        }
        
        if(register_chrdev_region(newNode->dec_dev, 1 , dec_name)){
            printk(KERN_ALERT "decrypt: failed to register chardev region\n");
        }else{
            printk(KERN_ALERT "decrypt: registered chardev region!\n");    
        }
        
        
        //allocating cdev space
        newNode -> enc_cdev = cdev_alloc();
        newNode -> enc_cdev -> ops = enc_fop;
        cdev_init(newNode->enc_cdev, enc_fop);
        if(cdev_add(newNode->enc_cdev, newNode->enc_dev, 1)  < 0){
            printk(KERN_ALERT "encrypt: failed to add cdev\n");
        }else{
            printk(KERN_ALERT "encrypt: added cdev\n");
        }

        newNode -> dec_cdev = cdev_alloc();
        newNode -> dec_cdev -> ops = dec_fop;
        cdev_init(newNode->dec_cdev, dec_fop);
        if(cdev_add(newNode->dec_cdev, newNode->dec_dev, 1)  < 0){
            printk(KERN_ALERT "decrypt: failed to add cdev\n");
        }else{
            printk(KERN_ALERT "decrypt: added cdev\n");
        }

        //creating class and device
        newNode->enc_class = class_create(THIS_MODULE, enc_name);
        device_create(newNode->enc_class, NULL, newNode->enc_dev, NULL, enc_name);

        newNode->dec_class = class_create(THIS_MODULE, dec_name);
        device_create(newNode->dec_class, NULL, newNode->dec_dev, NULL, dec_name);

        printk(KERN_ALERT "helpMe1\n");

        //create/populate struct attribute 
        attr = (struct attribute*)kmalloc(sizeof(struct attribute), GFP_KERNEL);
        //attr->owner = THIS_MODULE;
        attr->name = "config";
        //attr->name = (char*)kmalloc(sizeof(char)*7, GFP_KERNEL);
        //sprintf(attr->name, "%s", "config");
        attr->mode = S_IRWXU;

        printk(KERN_ALERT "helpMe2\n");
        
        //kobject shit
        newNode ->kobj = &(newNode->enc_cdev-> kobj);
        kobj = &(newNode->enc_cdev-> kobj); //variable name is easier to work with
        ktype = kobj -> ktype;
        

        attr_arr = (struct attribute**)kmalloc(sizeof(struct attribute*) * 2, GFP_KERNEL);
        attr_arr[0] = attr;
        
        zeroed = (struct attribute*)kmalloc(sizeof(struct attribute), GFP_KERNEL);
        zeroed->name = 0;
        zeroed->mode =0;

        attr_arr[1] = 0;  

        //mess around with ktype
        ktype->default_attrs = attr_arr;        
        //give this ktype some sysfs_ops
        sysfs_ops = (struct sysfs_ops*)kmalloc(sizeof(struct sysfs_ops), GFP_KERNEL);
        sysfs_ops->show = myShow;
        sysfs_ops->store = myStore;
        ktype->sysfs_ops = sysfs_ops;   

        printk(KERN_ALERT "helpMe5\n"); 


//      kobject_add(&(newNode->enc_cdev-> kobj), NULL, "crypt%d_key", idCounter);
    
        
        if(kobject_add(kobj, kobj->parent, "c%d_key", idCounter) < 0){
            printk(KERN_ALERT "kobject_add broke\n");
        }else{
            printk(KERN_ALERT "kobject_add worked?\n");
        }


        printk(KERN_ALERT "helpMe6\n"); 

        //int sysfs_create_file(struct kobject *kobj, struct attribute *attr);
        //sysfs_create_file(&(newNode->enc_cdev-> kobj), attr);
        printk(KERN_ALERT "helpMe7\n");  

        //increment global idCounter
        idCounter++;

        return idCounter -1;     
}




void ioctl_get_key(int id){
    pairNode* itr = head -> next;
    printk(KERN_ALERT "in ioctl_get_key\n");
    while(itr != NULL){
        if(itr -> id == id){
            printk(KERN_ALERT "current key of device %d is %s\n", id, itr->key);
            //printk(KERN_ALERT "simulating get_key\n");
            break;
        }
        itr = itr -> next;
    }
    
}

void ioctl_change_key(int id, char* key){
    char* newKey;
    pairNode* itr = head -> next;

    while(itr != NULL){
        if(itr -> id == id){
            newKey = (char*)kmalloc(sizeof(char)*21, GFP_KERNEL);
            sprintf(newKey, "%s", key);
            itr -> key = newKey;
            break;
        }
        itr = itr -> next;
    }
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
        if(itr -> id == id){
            //remove itr from list
            itr_tmp -> next = itr_tmp -> next -> next;
            
            //remove kobject things
            kobject_del(itr-> kobj);

            //removing device bookkeeping                        
            device_destroy(itr->enc_class, itr->enc_dev);
            class_destroy(itr->enc_class);    
            unregister_chrdev_region(itr->enc_dev, 1);
            cdev_del(itr->enc_cdev);

            device_destroy(itr->dec_class, itr->dec_dev);
            class_destroy(itr->dec_class);    
            unregister_chrdev_region(itr->dec_dev, 1);
            cdev_del(itr->dec_cdev);
            //vvfree node & string
            //vfree(itr -> key);
            //vfree(itr);

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
    pairNode* f = head;
    pairNode* t = head -> next;
    while(t != NULL){
        printk("LOOPZ\n");
        
        //remove kobject things
        kobject_del(t-> kobj);

        //char dev info
        device_destroy(t->enc_class, t->enc_dev);
        class_destroy(t->enc_class);    
        unregister_chrdev_region(t->enc_dev, 1);
        cdev_del(t->enc_cdev);
       
        device_destroy(t->dec_class, t->dec_dev);
        class_destroy(t->dec_class);    
        unregister_chrdev_region(t->dec_dev, 1);
        cdev_del(t->dec_cdev);
 
        printk(KERN_ALERT "deleted sub-device\n");
        f = t;
        t = t -> next;
        
        //vfree node stuff 
        //vfree(f -> key);
        //vfree(f);
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


