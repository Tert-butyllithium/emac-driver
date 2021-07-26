#include "include/common.h"

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
	sz += size;
	return (void*) buf + sz-size;
}

void *calloc(size_t nelem, size_t elsize){
	static unsigned char buf[16384];
	static unsigned int sz = 0;
	void* res = (void*) buf + sz;
	sz += nelem * elsize;
	return res;
}


void* memalign(unsigned long long alignment, unsigned long long size){
	static uchar buf[16384];
	static unsigned int sz = 0;

	uchar* bufp = buf + sz;
	bufp = (uchar*) memalign_roundup(alignment, (unsigned long long) bufp);
	sz = (bufp - buf) + size;
	return (void*) (buf + sz - size);
}

void env_set(char* name, const char* val){
	printf("env_set: name: %s, val: %s",name,val?val:"NULL");
}

void udelay(unsigned long usec){
    int i=0;
    for(; i < 1000; i++){
        asm volatile("ADDI x0, x0, 0");
    }
}

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
        asm volatile("addi x0, x0, 0");

	asm volatile (".long 0x01b0000b");
	/* flush_dcache_all(); */
}
