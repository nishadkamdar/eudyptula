#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/string.h>
#include <linux/textsearch.h>

static struct ts_config *conf;
static struct ts_state eustate;

static unsigned int eudyptula_hook(void *priv, struct sk_buff *pskb,
				   const struct nf_hook_state *state)
{
	unsigned int pos;
	int i;
	/*pos = textsearch_find_continuous(conf, &eustate, pskb->data, pskb->len);
	if (pos != UINT_MAX) {
		printk(KERN_INFO "the pos is %d\n", pos);
		printk(KERN_INFO "Nishad Kamdar at %d\n", pos);
	}*/	

	/*for (i = 0 ; i < pskb->len; i++)
		printk(KERN_INFO "%c", pskb->data[i]);*/
	for(pos = skb_find_text(pskb, 0, INT_MAX, conf);
	    pos != UINT_MAX;
	    pos = textsearch_next(conf, &eustate))
	{
		printk(KERN_INFO "the pos is %d\n", pos);
		printk(KERN_INFO "Nishad Kamdar at %d\n", pos);
	}

	printk(KERN_INFO "Packet received of length %d\n", pskb->len);
	return NF_ACCEPT;
}

static struct nf_hook_ops eudyptula_ops = {
	.hook		= eudyptula_hook,
	.pf 		= AF_INET,
	.hooknum	= NF_INET_LOCAL_OUT,
	.priority	= NF_IP_PRI_FIRST,
};

static int __init init (void)
{
	printk(KERN_INFO "registering hook\n");

	conf = textsearch_prepare("kmp", "Nishad", 6, GFP_KERNEL, 1);
	if (IS_ERR(conf))
		return PTR_ERR(conf);

	return nf_register_net_hook(&init_net, &eudyptula_ops);
}

static void __exit fini(void)
{
	printk("unregistering hook\n");
	textsearch_destroy(conf);
	nf_unregister_net_hook(&init_net, &eudyptula_ops);
}

module_init(init);
module_exit(fini);
