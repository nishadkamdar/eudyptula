#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/string.h>
#include <linux/textsearch.h>

#define MY_ID "Nishad Kamdar"
static struct ts_config *conf;

static unsigned int eudyptula_hook(void *priv, struct sk_buff *pskb,
				   const struct nf_hook_state *state)
{
	int pos;
	struct ts_state eustate;

	memset(&eustate, 0, sizeof(struct ts_state));

	for(pos = skb_find_text(pskb, 0, INT_MAX, conf);
	    pos != UINT_MAX;
	    pos = textsearch_next(conf, &eustate))
		printk(KERN_INFO "Nishad Kamdar at %d\n", pos);

	return NF_ACCEPT;
}

static struct nf_hook_ops eudyptula_ops = {
	.hook		= eudyptula_hook,
	.pf 		= NFPROTO_IPV4,
	.hooknum	= NF_INET_LOCAL_IN,
	.priority	= NF_IP_PRI_FILTER-1,
};

static int __init init (void)
{
	int pos;
	struct ts_state eustate;

	conf = textsearch_prepare("kmp", MY_ID, 13, GFP_KERNEL, TS_AUTOLOAD);
	if (IS_ERR(conf))
		return PTR_ERR(conf);

	memset(&eustate, 0, sizeof(struct ts_state));

	for(pos = textsearch_find_continuous(conf, &eustate, );
	    pos != UINT_MAX;
	    pos = textsearch_next(conf, &eustate))
		printk(KERN_INFO "Nishad Kamdar at %d\n", pos);

}

static void __exit fini(void)
{
	textsearch_destroy(conf);
}

module_init(init);
module_exit(fini);
