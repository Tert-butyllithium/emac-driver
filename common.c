#include "include/common.h"
#include "net/net.h"
#include "include/ctype.h"

void __assert_fail(const char *assertion, const char *file, unsigned int line,
		   const char *function);
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

void __assert_fail(const char *assertion, const char *file, unsigned int line,
		   const char *function)
{
	printf("%s:%u: %s: Assertion `%s' failed.", file, line, function,
	      assertion);
}

// string.h 


int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static const char *_parse_integer_fixup_radix(const char *s, unsigned int *base)
{
	if (*base == 0) {
		if (s[0] == '0') {
			if (tolower(s[1]) == 'x' && isxdigit(s[2]))
				*base = 16;
			else
				*base = 8;
		} else
			*base = 10;
	}
	if (*base == 16 && s[0] == '0' && tolower(s[1]) == 'x')
		s += 2;
	return s;
}


unsigned long simple_strtoul(const char *cp, char **endp,
				unsigned int base)
{
	unsigned long result = 0;
	unsigned long value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

/**
 * memcmp - Compare two areas of memory
 * @cs: One area of memory
 * @ct: Another area of memory
 * @count: The size of the area.
 */
int memcmp(const void * cs,const void * ct,size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}


/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */
void * memcpy(void *dest, const void *src, size_t count)
{
	unsigned long *dl = (unsigned long *)dest, *sl = (unsigned long *)src;
	char *d8, *s8;

	if (src == dest)
		return dest;

	/* while all data is aligned (common case), copy a word at a time */
	if ( (((ulong)dest | (ulong)src) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}
	/* copy the reset one byte at a time */
	d8 = (char *)dl;
	s8 = (char *)sl;
	while (count--)
		*d8++ = *s8++;

	return dest;
}

/**
 * strncpy - Copy a length-limited, %NUL-terminated string
 * @dest: Where to copy the string to
 * @src: Where to copy the string from
 * @count: The maximum number of bytes to copy
 *
 * Note that unlike userspace strncpy, this does not %NUL-pad the buffer.
 * However, the result is not %NUL-terminated if the source exceeds
 * @count bytes.
 */
char * strncpy(char * dest,const char *src,size_t count)
{
	char *tmp = dest;

	while (count-- && (*dest++ = *src++) != '\0')
		/* nothing */;

	return tmp;
}

/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void * memset(void * s,int c,size_t count)
{
	unsigned long *sl = (unsigned long *) s;
	char *s8;
	unsigned long cl = 0;
	int i;

	/* do it one word at a time (32 bits or 64 bits) while possible */
	if ( ((ulong)s & (sizeof(*sl) - 1)) == 0) {
		for (i = 0; i < sizeof(*sl); i++) {
			cl <<= 8;
			cl |= c & 0xff;
		}
		while (count >= sizeof(*sl)) {
			*sl++ = cl;
			count -= sizeof(*sl);
		}
	}
	s8 = (char *)sl;
	while (count--)
		*s8++ = c;

	return s;
}


static unsigned long memalign_roundup(unsigned long alignment, unsigned long addr){
	unsigned long tmp = addr % alignment;
	if(tmp){
		return alignment + addr - tmp;
	}
	else{
		return addr;
	}
}

void* malloc(unsigned long long size){
	static unsigned char buf[16384];
	static unsigned int sz = 0;
    printf("malloc at %p\n", buf+sz);
	sz += size;
	return (void*) buf + sz-size;
}

void *calloc(size_t nelem, size_t elsize){
	static unsigned char buf[16384];
	static unsigned int sz = 0;
	void* res = (void*) buf + sz;
    printf("calloc at %p\n",res);
	sz += nelem * elsize;
	return res;
}


void* memalign(unsigned long long alignment, unsigned long long size){
	static uchar buf[16384];
	static unsigned int sz = 0;
	uchar* bufp = buf + sz;
    printf("memalign at %p",bufp);
	bufp = (uchar*) memalign_roundup(alignment, (unsigned long long) bufp);
	sz = (bufp - buf) + size;
	return (void*) (buf + sz - size);
}


#define L1_CACHE_BYTES CONFIG_SYS_CACHELINE_SIZE

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);
	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x0295000b");	/*dcache.cpa a0*/
	asm volatile(".long 0x01b0000b");		/*sync.is*/
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		// asm volatile ("dcache.ipa a0");
        asm volatile(".long  0x02a5000b");

	asm volatile (".long 0x01b0000b");
	/* flush_dcache_all(); */
}


struct in_addr string_to_ip(const char *s)
{
	struct in_addr addr;
	char *e;
	int i;

	addr.s_addr = 0;
	if (s == NULL)
		return addr;

	for (addr.s_addr = 0, i = 0; i < 4; ++i) {
		ulong val = s ? simple_strtoul(s, &e, 10) : 0;
		if (val > 255) {
			addr.s_addr = 0;
			return addr;
		}
		if (i != 3 && *e != '.') {
			addr.s_addr = 0;
			return addr;
		}
		addr.s_addr <<= 8;
		addr.s_addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	addr.s_addr = htonl(addr.s_addr);
	return addr;
}


static unsigned int y = 1U;

 unsigned int rand_r(unsigned int *seedp)
{
	*seedp ^= (*seedp << 13);
	*seedp ^= (*seedp >> 17);
	*seedp ^= (*seedp << 5);

	return *seedp;
}

static inline unsigned int rand(void)
{
	return rand_r(&y);
}

static inline void srand(unsigned int seed)
{
	y = seed;
}


/*
 * Return a seed for the PRNG derived from the eth0 MAC address.
 */
static inline unsigned int seed_mac(void)
{
	unsigned char enetaddr[ARP_HLEN];
	unsigned int seed;

	/* get our mac */
	memcpy(enetaddr, eth_get_ethaddr(), ARP_HLEN);

	seed = enetaddr[5];
	seed ^= enetaddr[4] << 8;
	seed ^= enetaddr[3] << 16;
	seed ^= enetaddr[2] << 24;
	seed ^= enetaddr[1];
	seed ^= enetaddr[0] << 8;

	return seed;
}

/*
 * Seed the random number generator using the eth0 MAC address.
 */
static inline void srand_mac(void)
{
	srand(seed_mac());
}