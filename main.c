
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/io.h>
#include "include/common.h"
#include "net/net.h"

MODULE_AUTHOR("Haonan Li");

u64 PHY_CLK_REG_REMAPPED;
u64 CCMU_BASE_REMAPPED;
u64 IOBASE_REMAPPED;

// struct in_addr net_ping_ip;
extern struct in_addr net_ping_ip;
static void do_ping(void)
{
	// if (argc < 2)
	// 	return CMD_RET_USAGE;
	char* ip = "192.168.100.3";
	net_ping_ip = string_to_ip("192.168.100.3");
	// if (net_ping_ip.s_addr == 0)
	// 	return CMD_RET_USAGE;

	if (net_loop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", ip);
		// return CMD_RET_FAILURE;
		return;
	}

	printf("host %s is alive\n", ip);

	// return CMD_RET_SUCCESS;
}

static void do_ioremap(void){
	PHY_CLK_REG_REMAPPED = ioremap(0x03000000, 4096) + 0x30;
	CCMU_BASE_REMAPPED = ioremap(CCMU_BASE,4096);
	IOBASE_REMAPPED = ioremap(IOBASE, 4096);
}

static void do_iounmap(void){
	iounmap(PHY_CLK_REG_REMAPPED);
	iounmap(CCMU_BASE_REMAPPED);
	iounmap(IOBASE_REMAPPED);
}


static int __init net_drv_init(void)
{
	// env_set("ipaddr", "192.168.100.233");
	net_ip = string_to_ip("192.168.100.233");
	// env_set("netmask", "255.255.255.0");
	net_netmask = string_to_ip("255.255.255.0");
	net_gateway = string_to_ip("192.168.100.3");


	printk(KERN_INFO "Hello world 1.\n");
	do_ioremap();
	eth_initialize();
	printf("\033[0;33m......eth_initialize finished......\n\033[0m");
	printf("eth_get_dev().name=%s\n",eth_get_dev()->name);
	do_ping();
	/* 
	 * A non 0 return means net_drv_init failed; module can't be loaded. 
	 */
	return 0;
}

static void __exit net_drv_exit(void)
{
	do_iounmap();
	printk(KERN_INFO "Goodbye world 1.\n");
}

module_init(net_drv_init);
module_exit(net_drv_exit);

MODULE_LICENSE("MIT");