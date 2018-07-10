#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>

struct task_struct *ts[2];
int flag[2] = { 0, 0 };

/* Load, wait 6 seconds, unload */
/* kthread_stop signals kthread_should_stop to exit the function */
/* A flag is needed to prevent kthread_stop from executing if the function
   has already completed - otherwise a kernel NULL reference will occur */
int function(void *data) {
   int ind, n=0;
   while (!(ind=kthread_should_stop()) && n++ < 50) msleep(100*((int)data+1));
   if (ind) printk("thread %d stopped by cleanup_module at %d\n", (int)data, n);
   else printk("thread %d completed normally\n",(int)data);
   flag[(int)data] = 1;
   return 0;
}

int init_module(void) {
   /* Start two kernel threads */
   printk(KERN_EMERG "Starting...");
   ts[0] = kthread_run(function, (void*)0, "spawn1");
   ts[1] = kthread_run(function, (void*)1, "spawn2");
   return 0;
}

void cleanup_module(void) {
   /* stop the kernel thread */
   if (!flag[0]) kthread_stop(ts[0]);
   if (!flag[1]) kthread_stop(ts[1]);
   printk(KERN_INFO "Module unload successful\n");
}
