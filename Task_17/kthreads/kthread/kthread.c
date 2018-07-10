#include <linux/module.h>
#include <linux/kthread.h>

struct task_struct *ts[2];
struct params { char name[100]; int delay; int idx; };
struct params threads[2];

/* The argument to function is void* to allow any arrangement of input  *
 * parameters into the function.  In this example a string and two ints *
 * are passed to function through struct params                         */
int function(void *data) {
   int delay = ((struct params*)data)->delay;
   set_current_state(TASK_INTERRUPTIBLE);
   schedule_timeout(delay*HZ);
   printk(KERN_INFO "%s: Ding", ((struct params*)data)->name);
   printk(KERN_INFO "thread %d stopped",(((struct params*)data)->idx+1));
   return 0;
}

int init_module(void) {
   /* Initialize the parameter structures */
   strncpy(threads[0].name,"thread 1 speaking",17);
   strncpy(threads[1].name,"thread 2 speaking",17);
   threads[0].delay = 2;
   threads[1].delay = 5;
   threads[0].idx = 0;
   threads[1].idx = 1;

   /* Create and start two kernel threads */
   printk(KERN_EMERG "Starting..");
   ts[0] = kthread_run(function, (void*)&threads[0], "spawn1");
   ts[1] = kthread_run(function, (void*)&threads[1], "spawn2");
   return 0;
}

void cleanup_module(void) {
   printk(KERN_EMERG "Module unload successful\n");
}

MODULE_LICENSE("GPL");
