#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#define PROCFS_NAME 		"sarlkm"

static char *prompt = "prompt";

module_param(prompt, charp, 0000);
MODULE_PARM_DESC(prompt, "prompt string");

int 
sar_proc_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
  return sprintf(buffer, "%s,Kernel Panic,%lu\n",prompt, jiffies/HZ);
}


static int __init sar_init(void)
{
  struct proc_dir_entry *my_proc_entry;
  my_proc_entry = create_proc_entry( PROCFS_NAME, S_IRUGO, NULL );
  my_proc_entry->read_proc = sar_proc_read;
  return 0;
}

static void __exit sar_cleanup(void)
{
	remove_proc_entry(PROCFS_NAME, NULL);
}

module_init(sar_init);
module_exit(sar_cleanup);

MODULE_LICENSE("GPL");
