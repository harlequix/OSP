#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#define PROCFS_NAME 		"sarlkm"
#define PROCFS_MAX_SIZE		1024
#include <asm/uaccess.h>
static unsigned long procfs_buffer_size = 0;

static char procfs_buffer[PROCFS_MAX_SIZE];
static char *prompt = "prompt";

module_param(prompt, charp, 0000);
MODULE_PARM_DESC(prompt, "prompt string");


int 
sar_proc_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
  return sprintf(buffer, "%s,Kernel Panic,%lu\n",procfs_buffer, jiffies/HZ);
}


int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	/* get buffer size */
	procfs_buffer_size = count;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	/* write data to the buffer */
	if ( copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_buffer_size;
}




static int __init sar_init(void)
{
  struct proc_dir_entry *sarlkm;
  sarlkm = create_proc_entry( PROCFS_NAME, S_IRUGO, NULL );
  sarlkm->read_proc = sar_proc_read;
  sarlkm->write_proc= procfile_write;
  
	/*sarlkm->owner 	  = THIS_MODULE;
	sarlkm->mode 	  = S_IFREG | S_IRUGO;
	sarlkm->uid 	  = 0;
	sarlkm->gid 	  = 0;
	sarlkm->size 	  = 37;*/
  return 0;
}

static void __exit sar_cleanup(void)
{
	remove_proc_entry(PROCFS_NAME, NULL);
}

module_init(sar_init);
module_exit(sar_cleanup);

MODULE_LICENSE("GPL");
