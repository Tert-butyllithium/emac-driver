// /* SPDX-License-Identifier: GPL-2.0+ */
// /*
//  * (C) Copyright 2000-2009
//  * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
//  */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

#define IOBASE                  0x04500000
#define PHY_CLK_REG             (0x03000000 + 0x30)  /* SYS_CFG_BASE + EMAC_EPHY_CLK_REG0 */
#define CCMU_BASE               0x02001000  /* CCMU Base Address */
#define CCMU_GMAC_CLK_REG       0x097c	/* GMAC_BGR_REG */
#define CCMU_GMAC_RST_BIT       16	/* GMAC_RST */
#define CCMU_GMAC_GATING_BIT    0	/* GMAC_GATING */
#define CCMU_EPHY_CLK_REG       0x0970	/* GMAC_25M_CLK_REG */
#define CCMU_EPHY_GATING_BIT    31	/* GMAC_25M_CLK_GATING */


#define puts(str)			\
	do {						\
		printf("%s\n",str);	\
	} while (0)
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
// #define printf printk
#define printf(fmt, args...) printk(KERN_INFO "[network drviver]:" fmt, ##args)
// #define printf(fmt, ...) printk(fmt, __VA_ARGS__)
#define debug printk
#elif
#include "../linux/types.h"
typedef unsigned long long size_t;
#endif
#define putc(chr) printf("%c",chr); 

enum env_op {
	env_op_create,
	env_op_delete,
	env_op_overwrite,
};

#define CACHE_LINE_SIZE (64)

#define debug_cond(cond, fmt, args...)			\
	do {						\
		if (cond)				\
			printf(pr_fmt(fmt), ##args);	\
	} while (0)


#define bootstage_mark_name(a,b)
#define bootstage_mark(a)
#define bootstage_error(a)

#define H_PROGRAMMATIC 	(1 << 9)
#define U_BOOT_ENV_CALLBACK(name, callback) 
#define env_set_hex(varname, value)
#define get_timer(x) (19260817)

#define ctrlc() (0)
#define env_get_yesno(x) (0)
#define get_env_id() (1)
#define free(x) 

int env_set(const char* name, const char* val);
char* env_get(const char* name);

#define WATCHDOG_RESET()

#define tftp_start(x)
#define bootp_reset()
#define bootp_request()
 
// typedef unsigned char u8;
// typedef unsigned short u16;
// typedef unsigned int u32;
// typedef unsigned long u64;

typedef unsigned char uchar;
typedef unsigned long ulong;



#ifndef NULL 
#define NULL ((char *)0)
#endif

#ifndef __always_unused
#define __always_unused                 __attribute__((__unused__))
#define __maybe_unused                  __attribute__((__unused__))
#endif

#define DECLARE_GLOBAL_DATA_PTR 

#ifndef __ASSEMBLY__
typedef struct bd_info {
	int		bi_baudrate;	/* serial console baudrate */
	unsigned long	bi_ip_addr;	/* IP Address */
	unsigned char	bi_enetaddr[6];	/* Ethernet adress */
	unsigned long	bi_boot_params;	/* where this board expects params */
	unsigned long	bi_memstart;	/* start of DRAM memory VA */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size  of FLASH memory */
	unsigned long	bi_flashoffset;	/* offset to skip UBoot image */
} bd_t;
#endif	/* __ ASSEMBLY__ */


void __assert_fail(const char *assertion, const char *file, unsigned int line,
		   const char *function);
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })


void * memcpy(void *dest, const void *src, size_t count);


void* malloc(unsigned long long size);

void *calloc(size_t nelem, size_t elsize);

void* memalign(unsigned long long alignment, unsigned long long size);

void udelay(unsigned long usec);

static inline void mdelay(unsigned long msec)
{
	while (msec--)
		udelay(1000);
}

void flush_dcache_range(unsigned long start, unsigned long end);

static inline void flush_cache(unsigned long addr, unsigned long size)
{
	/* invalidate_icache_range(addr, addr + size); */
	flush_dcache_range(addr, addr + size);
}

void invalidate_dcache_range(unsigned long start, unsigned long end);

#endif	/* __COMMON_H_ */

