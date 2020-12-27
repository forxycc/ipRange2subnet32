#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_VALUE	(4294967295)

typedef struct net_info_st
{
	char	ip[16];
	char	mask[16];
}net_info_t;

static inline uint32_t bit32_is_set(uint32_t v, int point)
{
	uint32_t a;

	assert(point >=0 && point <= 31);

	a = 1 << point;
	return (v & a);
}

static inline uint32_t bit32_is_set2(uint32_t v, int vpoint)
{
	uint32_t a;

	assert(vpoint >= 0 && vpoint <= 31);

	a = 1 << (31 - vpoint);
	return (v & a);
}

static inline void bit32_string(uint32_t v, char *out)
{
	uint32_t point, isset;

	for(point = 31; point >= 0; point--)
	{
		isset = bit32_is_set(v, point);
		if(isset)
		{
			strcat(out, "1");
		}
		else
		{
			strcat(out, "0");
		}
	}
}

static inline void bit32_print(uint32_t v)
{
	char	out[64] = {0};

	bit32_string(v, out);
	printf("%s\n", out);
}

/*
 *	input: 	xxx10010000
 *	output:	00000001111
 */
static inline uint32_t bit32_subnet(uint32_t a)
{
	uint32_t	i, isset, v;

	for(i = 0; i < 32; i++)
	{
		isset = bit32_is_set(a, i);
		if(isset)
		{
			break;
		}
	}
	v = 1 << i;
	v -= 1;
	return v;
}

/*
 *	73 return 64
 *	1001001 return 1000000
 */
static inline uint32_t bit32_max_powerof2(uint32_t a)
{
	uint32_t i,isset;

	for(i = 0; i < 32; i++)
	{
		isset = bit32_is_set2(a, i);
		if(isset)
		{
			break;
		}
	}
	return (1 << (31 - i));
}

static inline void uint32_ip(uint32_t a, char *ip)
{
	char	*str;
	struct in_addr addr;

	addr.s_addr = htonl(a);
	str = inet_ntoa(addr);
	strcpy(ip, str);
}

int	range2sub32(uint32_t r0, uint32_t r1, net_info_t **net) 
{
	int		subnum;
	uint32_t	n, subnet, prefix, r2;
	uint32_t	subnet_ip_num;
	char		str_prefix[64], str_mask[64];
	net_info_t	*netalloc, *netalloc2;
	uint64_t	outrange;

	if(r1 < r0)
	{
		return 0;
	}

	if(r0 == 0)
	{
		r0 = 1;
	}

	n = r1 - r0 + 1;
	subnet = bit32_subnet(r0);
	subnet_ip_num = subnet + 1;
	prefix = ~subnet;

	if(subnet_ip_num <= n)
	{
		uint32_ip(r0, str_prefix);
		uint32_ip(prefix, str_mask);

		netalloc = (net_info_t *)malloc(sizeof(net_info_t));
		assert(netalloc != NULL);

		strcpy(netalloc->ip, str_prefix);
		strcpy(netalloc->mask, str_mask);

		(*net) = netalloc;

		outrange = (uint64_t)r0 + (uint64_t)subnet_ip_num;
		if(outrange > MAX_VALUE)
		{
			return 1;
		}

		r2 = r0 + subnet_ip_num;
		subnum = range2sub32(r2, r1, &netalloc2); 
		if(subnum > 0)
		{
			netalloc = realloc(netalloc, (subnum + 1) * sizeof(net_info_t));
			assert(netalloc != NULL);
			memcpy(netalloc + 1, netalloc2, subnum * sizeof(net_info_t));
			free(netalloc2);
			(*net) = netalloc;
		}
		return subnum + 1;
	}
	else
	{
		subnet_ip_num = bit32_max_powerof2(n);
		subnet = subnet_ip_num - 1;
		prefix = ~subnet;

		uint32_ip(r0, str_prefix);
		uint32_ip(prefix, str_mask);

		netalloc = (net_info_t *)malloc(sizeof(net_info_t));
		assert(netalloc != NULL);

		strcpy(netalloc->ip, str_prefix);
		strcpy(netalloc->mask, str_mask);

		(*net) = netalloc;

		outrange = (uint64_t)r0 + (uint64_t)subnet_ip_num;
		if(outrange > MAX_VALUE)
		{
			return 1;
		}

		r2 = r0 + subnet_ip_num;
		subnum = range2sub32(r2, r1, &netalloc2);
		if(subnum > 0)
		{
			netalloc = realloc(netalloc, (subnum + 1) * sizeof(net_info_t));
			assert(netalloc != NULL);
			memcpy(netalloc + 1, netalloc2, subnum * sizeof(net_info_t));
			free(netalloc2);
			(*net) = netalloc;
		}
		return subnum + 1;
	}
}

int	main(int argc, char **argv)
{
	int	i, num;
	uint32_t start, end;
	net_info_t *net;

	if(argc != 3)
	{
		printf("usage: range2sub32 192.168.1.100 192.168.1.200\n");
		return -1;
	}

	start = inet_addr(argv[1]); 
	start = ntohl(start);
	end = inet_addr(argv[2]);
	end = ntohl(end);

	assert(end >= start);
	num = range2sub32(start, end, &net);
	for(i = 0; i < num; i++)
	{
		printf("%s/%s\n", (net+i)->ip, (net+i)->mask);
	}
	free(net);
	return 0;
}
