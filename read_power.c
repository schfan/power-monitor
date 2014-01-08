#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/tty.h>      /* console_print() interface */
#include <asm/io.h> 
#include <linux/kthread.h> //for threads
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/delay.h>

// SPC_SYS_CFG registers
#define SPC_SYS_CFGCTRL ((unsigned int*) 0x7FFF0B10)
#define SPC_SYS_CFGREQ ((unsigned int*) 0x7FFF0B14)
#define PWC_STATUS ((unsigned int*) 0x7FFF0B18)
#define PWC_FLAG ((unsigned int*) 0x7FFF0B1C)
#define SPC_SYS_CFGWDATA ((unsigned int*) 0x7FFF0B70)
#define SPC_SYS_CFGRDATA ((unsigned int*) 0x7FFF0B74)

static struct task_struct *thread1;

int thread_fn() {
	unsigned int din,stat;
	int *mmio = NULL;
	int *cfgctrl = NULL;
	int *cfgreq = NULL;
	int *status = NULL;
	int *flag = NULL;
	int *cfgwdata = NULL;
	int *cfgrdata = NULL;
	int power15 = 0; 
	mmio = ioremap_nocache(0x7FFF0B00,0x100);
	if (mmio != NULL) {
		cfgctrl = (int *)((int)mmio + 0x10);
		cfgreq = (int *)((int)mmio + 0x14);
		status = (int *)((int)mmio + 0x18);
		flag = (int *)((int)mmio + 0x1C);
		cfgwdata = (int *)((int)mmio + 0x70);
		cfgrdata = (int *)((int)mmio + 0x74);
	} else {
		iounmap(mmio);
		mmio = NULL;
		return 0;
        } 
/*
	if ((*cfgreq & 0x00000001) ||
			(*flag & 0x00000001))
	{
		// Wait for complete flag (or interrupt 95)
		if (0){
			while (!(*flag & 0x00000001))
				continue;
		}
		// Handle any outstanding requests here
		// Clear the complete flag
		// Wait for complete flag (or interrupt 95)
	}
	// Set the data (only required for writes)
	*cfgwdata = 0x00000000;
	// Set the control value
	*cfgctrl = 0x80100001;
	// Wait for complete flag (or interrupt 95)
	if (0){
		while (!(*flag & 0x00000001))
			continue;
        }
	// Read value back
	din = *cfgrdata;
	// Read the status and clear the complete flag
	stat = *status;
	printk("cfgctrl:%x\n",*cfgctrl);
	printk("cfgreq:%x\n",*cfgreq);
	printk("status:%x\n",*status);
	printk("flag:%x\n",*flag);
	printk("cfgwdata:%x\n",*cfgwdata);
	printk("cfgrdata:%x\n",*cfgrdata);
	printk("\n");
	// Display value and status
	printk ("Data:0x%08X, Status:0x%08X\n", din, stat);
*/

	while (!kthread_should_stop())  {
             set_current_state(TASK_RUNNING);
	     *cfgctrl = 0x80c00000; //Read power for A15
             if (*cfgctrl == 0x80c00000){
                     power15 = *cfgrdata;
             }  
	
	     printk ("0x%08X\n", power15);
             set_current_state(TASK_INTERRUPTIBLE);
             msleep_interruptible(100);
	}

	iounmap(mmio);
	mmio = NULL;
	cfgctrl = NULL;
	cfgreq = NULL;
	status = NULL;
	flag = NULL;
	cfgwdata = NULL;
	cfgrdata = NULL;
	return 0;
}

int thread_init(void) {
	char this_thread[8]="readpw";
	printk(KERN_INFO "in readpw init");
	thread1 = kthread_create(thread_fn, NULL, this_thread);
	if (thread1){
		printk(KERN_INFO "in if");
		wake_up_process(thread1);
	}
	return 0;
}

void thread_cleanup(void)
{
	kthread_stop(thread1);
}

MODULE_LICENSE("GPL");
module_init(thread_init);
module_exit(thread_cleanup);
