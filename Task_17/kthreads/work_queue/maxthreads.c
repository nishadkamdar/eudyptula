#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

struct task_struct *ts[1000];
struct workqueue_struct *queue;
struct delayed_work dwork;
int count = 1;

int threadf (void *data) {
   while (!kthread_should_stop()) {  yield(); }
   return 0;
}

void function(struct work_struct *work) {
   ts[count] = kthread_run(threadf, NULL, "thread");
   printk(KERN_EMERG "Ding %d",count++);
   queue_delayed_work(queue, &dwork, HZ);
}

int init_module () {
   int i;
   for (i=0 ; i < 1000 ; i++) ts[i] = NULL;
   queue = create_workqueue("queue");
   INIT_DELAYED_WORK(&dwork, function);
   queue_delayed_work(queue, &dwork, HZ);
   printk(KERN_INFO "Module loaded successfuly");
   return 0;
}

void cleanup_module () {
   int i;
   flush_workqueue(queue);
   if (delayed_work_pending(&dwork)) {
      cancel_delayed_work(&dwork);
      flush_workqueue(queue);
   }
   destroy_workqueue(queue);
   for (i=0 ; i < 1000 ; i++) if (ts[i] != NULL) kthread_stop(ts[i]);
   printk(KERN_INFO "Module removed successfully");
}
