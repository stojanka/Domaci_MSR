#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(writeQueue);
DECLARE_WAIT_QUEUE_HEAD(readQueue);
struct semaphore sem;
char stred[100]; //realloc
int endRead = 0;

int stred_open(struct inode *pinode, struct file *pfile);
int stred_close(struct inode *pinode, struct file *pfile);
ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = stred_open,
	.read = stred_read,
	.write = stred_write,
	.release = stred_close,

};

int stred_open(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully opened stred\n");
		return 0;
}

int stred_close(struct inode *pinode, struct file *pfile)
{
		printk(KERN_INFO "Succesfully closed stred\n");
		return 0;
}

ssize_t stred_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	char *buff = (char *)kmalloc(length, GFP_KERNEL); //*buff je niz pokazivaca na char
  char string[] = "string=";
  char clear[] = "clear";
  char shrink[] = "shrink";
  char append[]= "append=";
  char truncate[]="truncate=";
  char remove[]="remove";

  char stred_cpy[100];

  int ret;
  int i, j;

  ret = copy_from_user(buff, buffer, length);
  if(ret)
  		return -EFAULT;
  buff[length-1] = '\0';


  if(strncmp(buff, string, strlen(string)) == 0){
    //ret = sscanf(buff, "%s", buff+strlen(string));
    //buff = buff + strlen(string);

    strsep(&buff, "=");
		if (strlen(stred)+strlen(buff)>99)
			printk(KERN_ERR "String ce premasiti 100karaktera! Cekam prazno mesto...\n");

		if(wait_event_interruptible(writeQueue,(strlen(stred)+strlen(buff)<=99)))
			return -ERESTARTSYS;

		strcpy(stred, buff);

  }

  if(strncmp(buff, clear, strlen(clear)) == 0){

    for(i=0; i< strlen(stred); i++){
      stred[i] = '0';
			wake_up_interruptible(&writeQueue);
    }
  }

  if(strncmp(buff, shrink, strlen(shrink)) == 0){
		strcpy(stred_cpy, stred);
    strim(stred);

		//provjera da li se oslobodilo dovoljno prostora za upis stringa koji ceka
		if(strlen(buff)<=99){
			if(strlen(stred_cpy)-strlen(stred) == strlen(buff)){
				wake_up_interruptible(&writeQueue);
			}
		}
  }

  if(strncmp(buff, append, strlen(append)) == 0){

    strsep(&buff, "=");
		if (strlen(stred)+strlen(buff)>99)
			printk(KERN_ERR "String ce premasiti 100karaktera! Cekam prazno mesto...\n");

		if(wait_event_interruptible(writeQueue,(strlen(stred)+strlen(buff)<=99)))
			return -ERESTARTSYS;


		strncat(stred, buff, strlen(buff));

  }

  if(strncmp(buff, truncate, strlen(truncate)) == 0){
		strsep(&buff, "=");
		kstrtoint(buff, 10, &ret);
		stred[strlen(stred)-ret] = '\0';
  }

  if(strncmp(buff, remove, strlen(remove)) == 0){
		strsep(&buff, "=");

		for(i=0; i< strlen(stred); i++){
			for(j=0; j < strlen(buff); j++){
				if(buff[j]==stred[j]){
						strreplace(stred, buff[j], ' ');
				}
			}
		}
//treba provjeriti da li je osobodjeno dovoljno prostora za upis u stred

  }




	wake_up_interruptible(&writeQueue);

printk("Buff je: %s \n", buff);
printk("Stred je: %s \n", stred);
  kfree(buff);
	return length;
}


ssize_t stred_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{

	if (endRead){
		endRead = 0;
		return 0;
	}

	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	while(strlen(stred) == 0)
	{
		up(&sem);
		if(wait_event_interruptible(readQueue,(strlen(stred)>0)))
			return -ERESTARTSYS;
		if(down_interruptible(&sem))
			return -ERESTARTSYS;
	}


return length;
}












static int __init stred_init(void)
{
  int ret = 0;
    int i=0;

    //sema_init(&sem,1);

    //Initialize array
    for (i=0; i<100; i++)
      stred[i] = 0;

  ret = alloc_chrdev_region(&my_dev_id, 0, 1, "stred");
  if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "stred_class");
   if (my_class == NULL){
     printk(KERN_ERR "failed to create class\n");
     goto fail_0;
   }
   printk(KERN_INFO "class created\n");

   my_device = device_create(my_class, NULL, my_dev_id, NULL, "stred");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
    printk(KERN_INFO "device created\n");

        my_cdev = cdev_alloc();
      	my_cdev->ops = &my_fops;
      	my_cdev->owner = THIS_MODULE;
      	ret = cdev_add(my_cdev, my_dev_id, 1);
      	if (ret)
      	{
      printk(KERN_ERR "failed to add cdev\n");
      		  goto fail_2;
      	}
         printk(KERN_INFO "cdev added\n");
         printk(KERN_INFO "Hello world\n");

         return 0;

         fail_2:
            device_destroy(my_class, my_dev_id);
         fail_1:
            class_destroy(my_class);
         fail_0:
            unregister_chrdev_region(my_dev_id, 1);
         return -1;
      }

static void __exit stred_exit(void)
{
  cdev_del(my_cdev);
  device_destroy(my_class, my_dev_id);
  class_destroy(my_class);
  unregister_chrdev_region(my_dev_id,1);
  printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(stred_init);
module_exit(stred_exit);
