/*
 *	Copied from LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Paolo Scaffardi
 */

#ifndef __NET_RAND_H__
#define __NET_RAND_H__

#include "../include/common.h"
#include "net.h"

static unsigned int y = 1U;

unsigned int rand_r(unsigned int *seedp)
{
	*seedp ^= (*seedp << 13);
	*seedp ^= (*seedp >> 17);
	*seedp ^= (*seedp << 5);

	return *seedp;
}

unsigned int rand(void)
{
	return rand_r(&y);
}

void srand(unsigned int seed)
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

#endif /* __NET_RAND_H__ */
