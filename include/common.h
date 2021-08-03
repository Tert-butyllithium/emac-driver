// /* SPDX-License-Identifier: GPL-2.0+ */
// /*
//  * (C) Copyright 2000-2009
//  * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
//  */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#define pr_fmt(fmt) "drv_net: " fmt
// #define CONFIG_MACH_SUN20IW1

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



#ifdef __KERNEL__
// #include <linux/kernel.h>
// #include <linux/module.h>
// #include <linux/types.h>


// #define printf printk
#define printf(fmt, args...) printk(KERN_INFO "[network drviver]:" fmt, ##args)
// #define printf(fmt, ...) printk(fmt, __VA_ARGS__)
#define debug printk
#else
#include <stddef.h>
#include "../linux/types.h"

#include "autoconf.h"
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))


# define do_div(n,base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
})


#define prink printd
// typedef unsigned char u8;
// typedef unsigned short u16;
// typedef unsigned int u32;
// typedef unsigned long u64;

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned int uint;

#ifdef __GNUC__
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif

typedef __s8  s8;
typedef __u8  u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;


#define debug(fmt, args...) printf(pr_fmt(fmt), ##args)

#define __weak  __attribute__((weak)) 
#define __always_inline __inline __attribute__((__always_inline__))


# define __iomem
#define printf printd
#define printk printd
#define panic(x)

# define likely(x)	 __builtin_expect(!!(x), 1)            
# define unlikely(x)	__builtin_expect(!!(x), 0)

#define BUG() do { \
	printk("BUG at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	panic("BUG!"); \
} while (0)

#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while (0)

#define printd(s, ...) 

// string.h

int strcmp(const char* s1, const char* s2);
unsigned long simple_strtoul(const char *cp, char **endp,
				unsigned int base);

int memcmp(const void * cs,const void * ct,size_t count);
char * strncpy(char * dest,const char *src,size_t count);
void * memset(void * s,int c,size_t count);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
size_t strlen(const char * s);
char * strchr(const char * s, int c);
#define strcpy(dest, src) strncpy(dest, src, 2147483647)

#include "byteorder.h"



#endif // endif __KERNEL__
#define putc(chr) printf("%c", chr)
#define puts(str) printf("%s\n", str)


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

ulong get_timer(ulong base);

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

ulong	get_tbclk     (void);

/* arch/$(ARCH)/lib/ticks.S */
uint64_t get_ticks(void);
void	wait_ticks    (unsigned long);

/* arch/$(ARCH)/lib/time.c */
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);


#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-4085)

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

static inline int generic_ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

# define ffs generic_ffs


#endif	/* __COMMON_H_ */

