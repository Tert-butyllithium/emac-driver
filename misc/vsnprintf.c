/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 *
 * from hush: simple_itoa() was lifted from boa-0.93.15
 */

#include <stdarg.h>
#include "../include/common.h"
#include "../include/ctype.h"

DECLARE_GLOBAL_DATA_PTR;

#define noinline __attribute__((noinline))

/**
 * strnlen - Find the length of a length-limited string
 * @s: The string to be sized
 * @count: The maximum number of bytes to search
 */
static size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

/* we use this so that we can do without the ctype library */
#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i = 0;

	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';

	return i;
}

/* Decimal conversion is by far the most typical, and is used
 * for /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed
 * using code from
 * http://www.cs.uiowa.edu/~jones/bcd/decimal.html
 * (with permission from the author, Douglas W. Jones). */

/* Formats correctly any integer in [0,99999].
 * Outputs from one to five digits depending on input.
 * On i386 gcc 4.1.2 -O2: ~250 bytes of code. */
static char *put_dec_trunc(char *buf, unsigned q)
{
	unsigned d3, d2, d1, d0;
	d1 = (q>>4) & 0xf;
	d2 = (q>>8) & 0xf;
	d3 = (q>>12);

	d0 = 6*(d3 + d2 + d1) + (q & 0xf);
	q = (d0 * 0xcd) >> 11;
	d0 = d0 - 10*q;
	*buf++ = d0 + '0'; /* least significant digit */
	d1 = q + 9*d3 + 5*d2 + d1;
	if (d1 != 0) {
		q = (d1 * 0xcd) >> 11;
		d1 = d1 - 10*q;
		*buf++ = d1 + '0'; /* next digit */

		d2 = q + 2*d2;
		if ((d2 != 0) || (d3 != 0)) {
			q = (d2 * 0xd) >> 7;
			d2 = d2 - 10*q;
			*buf++ = d2 + '0'; /* next digit */

			d3 = q + 4*d3;
			if (d3 != 0) {
				q = (d3 * 0xcd) >> 11;
				d3 = d3 - 10*q;
				*buf++ = d3 + '0';  /* next digit */
				if (q != 0)
					*buf++ = q + '0'; /* most sign. digit */
			}
		}
	}
	return buf;
}
/* Same with if's removed. Always emits five digits */
static char *put_dec_full(char *buf, unsigned q)
{
	/* BTW, if q is in [0,9999], 8-bit ints will be enough, */
	/* but anyway, gcc produces better code with full-sized ints */
	unsigned d3, d2, d1, d0;
	d1 = (q>>4) & 0xf;
	d2 = (q>>8) & 0xf;
	d3 = (q>>12);

	/*
	 * Possible ways to approx. divide by 10
	 * gcc -O2 replaces multiply with shifts and adds
	 * (x * 0xcd) >> 11: 11001101 - shorter code than * 0x67 (on i386)
	 * (x * 0x67) >> 10:  1100111
	 * (x * 0x34) >> 9:    110100 - same
	 * (x * 0x1a) >> 8:     11010 - same
	 * (x * 0x0d) >> 7:      1101 - same, shortest code (on i386)
	 */

	d0 = 6*(d3 + d2 + d1) + (q & 0xf);
	q = (d0 * 0xcd) >> 11;
	d0 = d0 - 10*q;
	*buf++ = d0 + '0';
	d1 = q + 9*d3 + 5*d2 + d1;
		q = (d1 * 0xcd) >> 11;
		d1 = d1 - 10*q;
		*buf++ = d1 + '0';

		d2 = q + 2*d2;
			q = (d2 * 0xd) >> 7;
			d2 = d2 - 10*q;
			*buf++ = d2 + '0';

			d3 = q + 4*d3;
				q = (d3 * 0xcd) >> 11; /* - shorter code */
				/* q = (d3 * 0x67) >> 10; - would also work */
				d3 = d3 - 10*q;
				*buf++ = d3 + '0';
					*buf++ = q + '0';
	return buf;
}


# define do_div(n,base) ({					\
	uint32_t __base = (base);				\
	uint32_t __rem;						\
	__rem = ((uint64_t)(n)) % __base;			\
	(n) = ((uint64_t)(n)) / __base;				\
	__rem;							\
})


/* No inlining helps gcc to use registers better */
static noinline char *put_dec(char *buf, uint64_t num)
{
	while (1) {
		unsigned rem;
		if (num < 100000)
			return put_dec_trunc(buf, num);
		rem = do_div(num, 100000);
		buf = put_dec_full(buf, rem);
	}
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */

/*
 * Macro to add a new character to our output string, but only if it will
 * fit. The macro moves to the next character position in the output string.
 */
#define ADDCH(str, ch) do { \
	if ((str) < end) \
		*(str) = (ch); \
	++str; \
	} while (0)


static char *number(char *buf, char *end, u64 num,
		int base, int size, int precision, int type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF";

	char tmp[66];
	char sign;
	char locase;
	int need_pfx = ((type & SPECIAL) && base != 10);
	int i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	sign = 0;
	if (type & SIGN) {
		if ((s64) num < 0) {
			sign = '-';
			num = -(s64) num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (need_pfx) {
		size--;
		if (base == 16)
			size--;
	}

	/* generate full string in tmp[], in reverse order */
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	/* Generic code, for any base:
	else do {
		tmp[i++] = (digits[do_div(num,base)] | locase);
	} while (num != 0);
	*/
	else if (base != 10) { /* 8 or 16 */
		int mask = base - 1;
		int shift = 3;

		if (base == 16)
			shift = 4;

		do {
			tmp[i++] = (digits[((unsigned char)num) & mask]
					| locase);
			num >>= shift;
		} while (num);
	} else { /* base 10 */
		i = put_dec(tmp, num) - tmp;
	}

	/* printing 100 using %2d gives "100", not "00" */
	if (i > precision)
		precision = i;
	/* leading space padding */
	size -= precision;
	if (!(type & (ZEROPAD + LEFT))) {
		while (--size >= 0)
			ADDCH(buf, ' ');
	}
	/* sign */
	if (sign)
		ADDCH(buf, sign);
	/* "0x" / "0" prefix */
	if (need_pfx) {
		ADDCH(buf, '0');
		if (base == 16)
			ADDCH(buf, 'X' | locase);
	}
	/* zero or space padding */
	if (!(type & LEFT)) {
		char c = (type & ZEROPAD) ? '0' : ' ';

		while (--size >= 0)
			ADDCH(buf, c);
	}
	/* hmm even more zero padding? */
	while (i <= --precision)
		ADDCH(buf, '0');
	/* actual digits of result */
	while (--i >= 0)
		ADDCH(buf, tmp[i]);
	/* trailing space padding */
	while (--size >= 0)
		ADDCH(buf, ' ');
	return buf;
}

static char *string(char *buf, char *end, char *s, int field_width,
		int precision, int flags)
{
	int len, i;

	if (s == NULL)
		s = "<NULL>";

	len = strnlen(s, precision);

	if (!(flags & LEFT))
		while (len < field_width--)
			ADDCH(buf, ' ');
	for (i = 0; i < len; ++i)
		ADDCH(buf, *s++);
	while (len < field_width--)
		ADDCH(buf, ' ');
	return buf;
}

uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size)
{
	uint32_t code_high = 0;

	while (size--) {
		uint32_t code = *src++;

		if (code_high) {
			if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Surrogate pair.  */
				code = ((code_high - 0xD800) << 10) + (code - 0xDC00) + 0x10000;

				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				/* Error...  */
				*dest++ = '?';
				/* *src may be valid. Don't eat it.  */
				src--;
			}

			code_high = 0;
		} else {
			if (code <= 0x007F) {
				*dest++ = code;
			} else if (code <= 0x07FF) {
				*dest++ = (code >> 6) | 0xC0;
				*dest++ = (code & 0x3F) | 0x80;
			} else if (code >= 0xD800 && code <= 0xDBFF) {
				code_high = code;
				continue;
			} else if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Error... */
				*dest++ = '?';
			} else if (code < 0x10000) {
				*dest++ = (code >> 12) | 0xE0;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			}
		}
	}

	return dest;
}


#if defined(CONFIG_EFI_LOADER) && \
	!defined(CONFIG_SPL_BUILD) && !defined(API_BUILD)
static char *device_path_string(char *buf, char *end, void *dp, int field_width,
				int precision, int flags)
{
	u16 *str;

	/* If dp == NULL output the string '<NULL>' */
	if (!dp)
		return string16(buf, end, dp, field_width, precision, flags);

	str = efi_dp_str((struct efi_device_path *)dp);
	if (!str)
		return ERR_PTR(-ENOMEM);

	buf = string16(buf, end, str, field_width, precision, flags);
	efi_free_pool(str);
	return buf;
}
#endif

static const char hex_asc[] = "0123456789abcdef";
static const char hex_asc_upper[] = "0123456789ABCDEF";
#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]
static inline char *hex_byte_pack(char *buf, u8 byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);
	return buf;
}


#ifdef CONFIG_CMD_NET
static char *mac_address_string(char *buf, char *end, u8 *addr, int field_width,
				int precision, int flags)
{
	/* (6 * 2 hex digits), 5 colons and trailing zero */
	char mac_addr[6 * 3];
	char *p = mac_addr;
	int i;

	for (i = 0; i < 6; i++) {
		p = hex_byte_pack(p, addr[i]);
		if (!(flags & SPECIAL) && i != 5)
			*p++ = ':';
	}
	*p = '\0';

	return string(buf, end, mac_addr, field_width, precision,
		      flags & ~SPECIAL);
}

static char *ip6_addr_string(char *buf, char *end, u8 *addr, int field_width,
			 int precision, int flags)
{
	/* (8 * 4 hex digits), 7 colons and trailing zero */
	char ip6_addr[8 * 5];
	char *p = ip6_addr;
	int i;

	for (i = 0; i < 8; i++) {
		p = hex_byte_pack(p, addr[2 * i]);
		p = hex_byte_pack(p, addr[2 * i + 1]);
		if (!(flags & SPECIAL) && i != 7)
			*p++ = ':';
	}
	*p = '\0';

	return string(buf, end, ip6_addr, field_width, precision,
		      flags & ~SPECIAL);
}

static char *ip4_addr_string(char *buf, char *end, u8 *addr, int field_width,
			 int precision, int flags)
{
	/* (4 * 3 decimal digits), 3 dots and trailing zero */
	char ip4_addr[4 * 4];
	char temp[3];	/* hold each IP quad in reverse order */
	char *p = ip4_addr;
	int i, digits;

	for (i = 0; i < 4; i++) {
		digits = put_dec_trunc(temp, addr[i]) - temp;
		/* reverse the digits in the quad */
		while (digits--)
			*p++ = temp[digits];
		if (i != 3)
			*p++ = '.';
	}
	*p = '\0';

	return string(buf, end, ip4_addr, field_width, precision,
		      flags & ~SPECIAL);
}
#endif


/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Right now we handle:
 *
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way (dot-separated
 *       decimal for v4 and colon separated network-order 16 bit hex for v6)
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses, IPv6 omits the colons, IPv4 is
 *       currently the same
 *
 * Note: The difference between 'S' and 'F' is that on ia64 and ppc64
 * function pointers are really function descriptors, which contain a
 * pointer to the real address.
 */
static char *pointer(const char *fmt, char *buf, char *end, void *ptr,
		int field_width, int precision, int flags)
{
	u64 num = (uintptr_t)ptr;

	/*
	 * Being a boot loader, we explicitly allow pointers to
	 * (physical) address null.
	 */
#if 0
	if (!ptr)
		return string(buf, end, "(null)", field_width, precision,
			      flags);
#endif

	switch (*fmt) {
#if defined(CONFIG_EFI_LOADER) && \
	!defined(CONFIG_SPL_BUILD) && !defined(API_BUILD)
	case 'D':
		return device_path_string(buf, end, ptr, field_width,
					  precision, flags);
#endif
#ifdef CONFIG_CMD_NET
	case 'a':
		flags |= SPECIAL | ZEROPAD;

		switch (fmt[1]) {
		case 'p':
		default:
			field_width = sizeof(phys_addr_t) * 2 + 2;
			num = *(phys_addr_t *)ptr;
			break;
		}
		break;
	case 'm':
		flags |= SPECIAL;
		/* Fallthrough */
	case 'M':
		return mac_address_string(buf, end, ptr, field_width,
					  precision, flags);
	case 'i':
		flags |= SPECIAL;
		/* Fallthrough */
	case 'I':
		if (fmt[1] == '6')
			return ip6_addr_string(buf, end, ptr, field_width,
					       precision, flags);
		if (fmt[1] == '4')
			return ip4_addr_string(buf, end, ptr, field_width,
					       precision, flags);
		flags &= ~SPECIAL;
		break;
#endif
// #ifdef CONFIG_LIB_UUID
// 	case 'U':
// 		return uuid_string(buf, end, ptr, field_width, precision,
// 				   flags, fmt);
// #endif
	default:
		break;
	}
	flags |= SMALL;
	if (field_width == -1) {
		field_width = 2*sizeof(void *);
		flags |= ZEROPAD;
	}
	return number(buf, end, num, 16, field_width, precision, flags);
}

static int vsnprintf_internal(char *buf, size_t size, const char *fmt,
			      va_list args)
{
	u64 num;
	int base;
	char *str;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */
				/* 'z' support added 23/7/1999 S.H.    */
				/* 'z' changed to 'Z' --davidm 1/25/99 */
				/* 't' added for ptrdiff_t */
	char *end = buf + size;

	/* Make sure end is always >= buf - do we want this in U-Boot? */
	if (end < buf) {
		end = ((void *)-1);
		size = end - buf;
	}
	str = buf;

	for (; *fmt ; ++fmt) {
		if (*fmt != '%') {
			ADDCH(str, *fmt);
			continue;
		}

		/* process flags */
		flags = 0;
repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
			case '-':
				flags |= LEFT;
				goto repeat;
			case '+':
				flags |= PLUS;
				goto repeat;
			case ' ':
				flags |= SPACE;
				goto repeat;
			case '#':
				flags |= SPECIAL;
				goto repeat;
			case '0':
				flags |= ZEROPAD;
				goto repeat;
			}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
		    *fmt == 'Z' || *fmt == 'z' || *fmt == 't') {
			qualifier = *fmt;
			++fmt;
			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				++fmt;
			}
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT)) {
				while (--field_width > 0)
					ADDCH(str, ' ');
			}
			ADDCH(str, (unsigned char) va_arg(args, int));
			while (--field_width > 0)
				ADDCH(str, ' ');
			continue;

		case 's':
			str = string(str, end, va_arg(args, char *),
				field_width, precision, flags);
			continue;

		case 'p':
			str = pointer(fmt + 1, str, end,
					va_arg(args, void *),
					field_width, precision, flags);
			if (IS_ERR(str))
				return ((long) str);
			/* Skip all alphanumeric pointer suffixes */
			while (isalnum(fmt[1]))
				fmt++;
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			ADDCH(str, '%');
			continue;

		/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			ADDCH(str, '%');
			if (*fmt)
				ADDCH(str, *fmt);
			else
				--fmt;
			continue;
		}
		if (qualifier == 'L')  /* "quad" for 64 bit variables */
			num = va_arg(args, unsigned long long);
		else if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long) num;
		} else if (qualifier == 'Z' || qualifier == 'z') {
			num = va_arg(args, size_t);
		} else if (qualifier == 't') {
			num = va_arg(args, ptrdiff_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (signed short) num;
		} else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int) num;
		}
		str = number(str, end, num, base, field_width, precision,
			     flags);
	}

	if (size > 0) {
		ADDCH(str, '\0');
		if (str > end)
			end[-1] = '\0';
		--str;
	}
	/* the trailing null byte doesn't count towards the total */
	return str - buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt,
			      va_list args)
{
	return vsnprintf_internal(buf, size, fmt, args);
}

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int i;

	i = vsnprintf(buf, size, fmt, args);

	if (likely(i < size))
		return i;
	if (size != 0)
		return size - 1;
	return 0;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return i;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vscnprintf(buf, size, fmt, args);
	va_end(args);

	return i;
}

/**
 * Format a string and place it in a buffer (va_list version)
 *
 * @param buf	The buffer to place the result into
 * @param fmt	The format string to use
 * @param args	Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf. Use vsnprintf() or vscnprintf() in order to avoid
 * buffer overflows.
 *
 * If you're not already dealing with a va_list consider using sprintf().
 */
int vsprintf(char *buf, const char *fmt, va_list args)
{
	return vsnprintf_internal(buf, 2147483647, fmt, args);
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}

char *simple_itoa(ulong i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	static char local[22];
	char *p = &local[21];

	*p-- = '\0';
	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);
	return p + 1;
}

/* We don't seem to have %'d in U-Boot */
void print_grouped_ull(unsigned long long int_val, int digits)
{
	char str[21], *s;
	int grab = 3;

	digits = (digits + 2) / 3;
	sprintf(str, "%*llu", digits * 3, int_val);
	for (s = str; *s; s += grab) {
		if (s != str)
			putc(s[-1] != ' ' ? ',' : ' ');
		printf("%.*s", grab, s);
		grab = 3;
	}
}


bool str2long(const char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return *p != '\0' && *endptr == '\0';
}
