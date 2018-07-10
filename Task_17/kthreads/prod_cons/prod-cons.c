#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include "queue_a.h"

#define BUFFER_SIZE 5
int idcnt = 1;
int mult1 = 3, mult2 = 5;

/* One of these per stream.
   Holds:  the mutex lock, and full and empty semaphores;
           a buffer of tokens taken from a producer
           a structure with information regarding the processing of tokens 
           an identity
*/
typedef struct stream_struct {
   struct stream_struct *next;
   struct semaphore empty;    /* if empty -> 0, capacity of queue is reached */
   struct semaphore full;     /* if full -> 0, no available elements in queue */
   struct semaphore mutex;
   queue buffer;   /* a buffer of values of any type      */
   void *args;     /* arguments for the producing stream  */
   int id;         /* identity of this stream             */
} Stream;

Stream suc1, suc2, tms1, tms2, mrg;

/* prod: linked list of streams that this producer consumes tokens from
   self: the producer's output stream */
typedef struct {
   Stream *self, *prod;
} Args;

Args suc1_args, suc2_args, tms1_args, tms2_args, mrg_args, cons_args;

/* return 'value' which should have a value from a call to put */
void *get(void *stream) {
   void *ret;
   queue *q = &((Stream*)stream)->buffer;
   struct semaphore *mutex = &((Stream*)stream)->mutex;
   struct semaphore *empty = &((Stream*)stream)->empty;
   struct semaphore *full = &((Stream*)stream)->full;

   /* if empty, wait until a token is available */
   /* if full goes to 0 then the buffer is empty */
   if (down_interruptible(full)) return NULL; 
   if (down_interruptible(mutex)) return NULL;
   ret = dequeue(q);
   up(mutex);
   up(empty);  /* empty increases as a buffer slot is freed */
               /* and a value is handed to a consumer */
   return ret;
}

/* 'value' is the value to move to the consumer */
int put(void *stream, void *value) {
   queue *q = &((Stream*)stream)->buffer;
   struct semaphore *empty = &((Stream*)stream)->empty;
   struct semaphore *full = &((Stream*)stream)->full;
   struct semaphore *mutex = &((Stream*)stream)->mutex;

   /* if full, wait for an open slot */
   /* if empty -> 0 then all slots are taken */
   if (down_interruptible(empty)) return -1;
   if (down_interruptible(mutex)) return -1;
   enqueue(q, value);  /* add another token (value) to the stream             */
   up(mutex);
   up(full);           /* full increases as a token is added to the buffer    */
                       /* a token has been successfully taken from a producer */
   return 0;
}

/* Put 1,2,3,4,5... into the self stream */
int successor (void *streams) {
   Stream *self = ((Args*)streams)->self;
   int id = self->id;
   queue *q = &self->buffer;
   int i, *value;
   
   for (i=1 ; ; i++) {
      msleep(100*self->id);
      printk("Successor(%d) (buff:%d): sending %d\n", id, q->nelem, i);

      value = (int*)vmalloc(sizeof(int));
      *value = i;
      put(self, value);
   }
   return 0;
}

/* multiply all tokens from the self stream by (int)self->args and insert
   the resulting tokens into the self stream */
int times (void *streams) {
   Stream *self = ((Args*)streams)->self;
   Stream *prod = ((Args*)streams)->prod;
   queue *q1 = &self->buffer;
   queue *q2 = &prod->buffer;
   int *value;
   
   printk("\t  Times(%d) connected to Successor (%d)\n", self->id, prod->id);
   while (true) {
      msleep(100);
      value = get(prod);

      printk("\t  Times(%d) (buff:%d): got %d from Successor %d (buff:%d)\n",
             self->id, q1->nelem, *value, prod->id, q2->nelem);

      *value = (*value) * (*(int*)(self->args));
      put(self, value);
   }
   return 0;
}

/* merge two streams that containing tokens in increasing order 
   ex: stream 1:  3,6,9,12,15,18...  stream 2: 5,10,15,20,25,30...
       output stream: 3,5,6,9,10,12,15,15,18...
*/
int merge (void *streams) {
   Stream *self = ((Args*)streams)->self;
   Stream *s1 = ((Args*)streams)->prod;
   Stream *s2 = (((Args*)streams)->prod)->next;
   queue *q1 = &s1->buffer;
   queue *q2 = &s2->buffer;
   queue *qs = &self->buffer;
   int *a = get(s1);
   int *b = get(s2);

   printk("\t\tMerge(%d) connected to Times (%d) and (%d)\n", 
	  self->id, s1->id, s2->id);
   while (true) {
      if (*a < *b) {
         put(self, a);
         a = get(s1);
	 printk("\t\tMerge(%d) (buff:%d): got %d from Times (%d) (buff:%d)\n",
		self->id, qs->nelem, *a, s1->id, q1->nelem);
      } else {
         put(self, b);
         b = get(s2);
	 printk("\t\tMerge(%d) (buff:%d): got %d from Times (%d) (buff:%d)\n",
		self->id, qs->nelem, *b, s2->id, q2->nelem);
      }
   }
   return 0;
}

/* Final consumer in the network */
int consumer (void *streams) {
   Stream *prod = ((Args*)streams)->prod;
   int i, *value;
   
   for (i=0 ; i < 10 ; i++) {
      printk("\t\t\t\t\tConsumer: got %d <---\n", *(value = get(prod)));
      vfree(value);
   }
   
   return 0;
}

/* initialize streams - see also queue_a.h and queue_a.c */
void init_stream (Args *args, Stream *self, void *data) {
   if (self != NULL) {
      self->next = NULL;
      self->args = data;
      self->id = idcnt++;
      init_queue(&self->buffer);
      sema_init(&self->mutex, 1);
      sema_init(&self->empty, BUFFER_SIZE);
      sema_init(&self->full, 0);
   }
   args->self = self;
   args->prod = NULL;
}

/* free allocated space in the queue - see queue_a.h and queue_a.c */
void kill_stream(Stream *stream) { destroy_queue(&stream->buffer); }

/* puts an initialized stream object onto the end of a stream's input list */
void connect (Args *arg, Stream *s) {  
   s->next = arg->prod;
   arg->prod = s;
}

int init_module () {
   struct task_struct *s1, *s2, *t1, *t2, *m1, *c1;

   printk("--------------------------------------------");

   init_stream(&suc1_args, &suc1, NULL);   /* initialize a successor stream */
   init_stream(&suc2_args, &suc2, NULL);   /* initialize a successor stream */
   init_stream(&tms1_args, &tms1, (void*)&mult1);   /* initialize x3 stream */
   connect(&tms1_args, &suc1);             /* connect to a successor        */

   init_stream(&tms2_args, &tms2, (void*)&mult2); /* initialize x5 stream   */
   connect(&tms2_args, &suc2);                    /* connect to a successor */

   init_stream(&mrg_args, &mrg, NULL);     /* initialize a merge stream     */
   connect(&mrg_args, &tms1);              /* connect to a times stream     */
   connect(&mrg_args, &tms2);              /* connect to a 2nd times stream */

   init_stream(&cons_args, NULL, NULL);    /* initialize a consumer stream  */
   connect(&cons_args, &mrg);              /* connect to a merge stream     */

   s1 = kthread_create(successor, (void*)&suc1_args, "suc_1");
   s2 = kthread_create(successor, (void*)&suc2_args, "suc_2");
   t1 = kthread_create(times, (void*)&tms1_args, "tms_1");
   t2 = kthread_create(times, (void*)&tms2_args, "tms_2");
   m1 = kthread_create(merge, (void*)&mrg_args, "mrg_1");
   c1 = kthread_create(consumer, (void*)&cons_args, "cons_1");

   wake_up_process(s1);
   wake_up_process(s2);
   wake_up_process(t1);
   wake_up_process(t2);
   wake_up_process(m1);
   wake_up_process(c1);

   printk(KERN_EMERG "Module up and running!\n");
   return 0;
}
   
void cleanup_module () { 
   printk(KERN_INFO "Final buffer sizes:");
   printk(KERN_INFO "  succ 1: %d",suc1.buffer.nelem);
   printk(KERN_INFO "  succ 2: %d",suc2.buffer.nelem);
   printk(KERN_INFO "  tmes 1: %d",tms1.buffer.nelem);
   printk(KERN_INFO "  tmes 2: %d",tms2.buffer.nelem);
   printk(KERN_INFO "  merge:  %d",mrg.buffer.nelem);
   kill_stream(&suc1);
   kill_stream(&suc2);
   kill_stream(&tms1);
   kill_stream(&tms2);
   kill_stream(&mrg);
   printk(KERN_INFO "Closing down successfully\n");
}
