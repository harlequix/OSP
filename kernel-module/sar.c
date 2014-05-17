#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#define PROCFS_NAME 		"sarlkm"
#define PROCFS_MAX_SIZE		1024
#include <asm/uaccess.h>
      #include <linux/sysctl.h>
            #include <linux/unistd.h>
static unsigned long procfs_buffer_size = 0;
static char prompt_param[PROCFS_MAX_SIZE];
//*prompt_param = "prompt";
//module_param(prompt_param, charp, 0000);
//MODULE_PARM_DESC(prompt_param, "prompt string");
static struct ctl_table_header *sar_table_header;

int 
sar_proc_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
  return sprintf(buffer, "%s,Kernel Panic,%lu\n",prompt_param, jiffies/HZ);
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
	if ( copy_from_user(prompt_param, buffer, procfs_buffer_size) ) {
		return -EFAULT;
	}
	
	return procfs_buffer_size;
}

static ctl_table test_table[] = {
        {
        .ctl_name       = CTL_UNNUMBERED,
        .procname       = "prompt",
        .data           = &prompt_param,
        .maxlen         = sizeof(char)*PROCFS_MAX_SIZE,
        .mode           = 0777,
        .proc_handler   = &proc_dostring,
        },{}
};
     


    static ctl_table test_root_table[] = {
            {
                    .ctl_name       = CTL_KERN,
                    .procname       = "kernel",
                    .mode           = 0555,
                    .child          = test_table
            },
            {}
    };



/*static struct ctl_table prompt_table[] = {
	 { .ctl_name = CTL_UNNUMBERED,
	  .procname = "prompt",
	  .maxlen = sizeof(int),
	  .mode = 0600,
	  .data=&procfs_buffer,
	  .proc_handler = &sar_proc_read,
	  //.extra1 = &min_val,
	  //.extra2 = &max_val,
	  }, {} 
};
static struct ctl_table sample_parent_table[] = {
	{ .ctl_name = CTL_KERN, .procname = "kernel",
	  .mode = 0555,
	  .child = prompt_table,
	}, {} 
};*/
	



static int __init sar_init(void)
{
  /*if (!register_sysctl_table(test_root_table)) {
	 return -EFAULT;
 }*/
 sar_table_header = register_sysctl_table(test_root_table);
  struct proc_dir_entry *sarlkm;
  sarlkm = create_proc_entry( PROCFS_NAME, 0666,NULL );
  sarlkm->read_proc = sar_proc_read;
  sarlkm->write_proc= procfile_write;
  
	//sarlkm->owner 	  = THIS_MODULE;
	//sarlkm->mode 	  = S_IFREG | S_IRUGO;
	//sarlkm->uid 	  = 0;
	//sarlkm->gid 	  = 0;
	//sarlkm->size 	  = 37;*/
  return 0;
}

static void __exit sar_cleanup(void)
{
	remove_proc_entry(PROCFS_NAME, NULL);
	unregister_sysctl_table(sar_table_header);
}
  

module_init(sar_init);
module_exit(sar_cleanup);

MODULE_LICENSE("GPL");
