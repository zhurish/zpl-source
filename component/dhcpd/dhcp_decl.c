/*
 * dhcp_decl.c
 *
 *  Created on: Sep 24, 2018
 *      Author: zhurish
 */

#define _GRP_H
#include "zebra.h"
#include "prefix.h"
#include "if.h"

#include <net/if.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcp.h"
#include "tree.h"
#include "dhcpd.h"
#include "dhctoken.h"
#include "dhcpd_utils.h"

extern struct interface_info *dhcpd_interfaces_list;


static int str_chr_count(unsigned char *input, int m)
{
	char *p = input;
	assert(input);
	int i = 0, j = 0, count = os_strlen(input);
	for(i = 0; i < count; i++)
	{
		if(p[i] == m)
		{
			j++;
		}
	}
	return j;
}

static int str_chr_next(char *src, const char em)
{
	char *p = src;
	assert(src);
	int i = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			if(i != 0)
				break;
		}
	}
	return i;
}

#ifdef DHCPD_TREE
static struct tree *dhcpd_host_addr_tree_lookup(char *name)
{
	struct hostent *h = NULL;
	struct tree *tree = NULL;
	struct in_addr addr;
	if (inet_aton(name, &addr))
	{
		tree = tree_const((unsigned char *) &addr.s_addr,
				sizeof(struct in_addr));
	}
	else
	{
		if (name)
			h = gethostbyname(name);
		if (name && h)
		{
			tree = tree_const(h->h_addr_list[0], h->h_length);
			tree = tree_limit(tree, strlen(name));
		}
	}
	if (!tree)
		return NULL;
	return tree;
}

static struct tree_cache *dhcpd_fixed_addr_tree_cache_lookup(struct iaddr *address,
		int num)
{
	int i = 0;
	struct tree *tree = NULL;
	struct tree *tmp = NULL;
	for (i = 0; i < num; i++)
	{
		tmp = tree_const(address[i].iabuf, address[i].len);
		if (tree)
			tree = tree_concat(tree, tmp);
		else
			tree = tmp;
	}
	return tree_cache(tree);
}
#endif


static int _dhcpd_option_address_split(char *input, unsigned char *output, int *len)
{
	unsigned char adval[64];
	unsigned char address[256];
	char *tokval = input;
	int offset = 0, num = 0;
	struct in_addr *addr = (struct in_addr *)address;
	num = str_chr_count(input, ',');
	if(num == 0)
	{
		if (inet_aton(input, addr))
		{
			if(output)
				memcpy(output, address, 4);
			if(len)
				*len = 4;
			return 0;
		}
		return -1;
	}
	while(offset < strlen(input))
	{
		offset = str_chr_next(tokval, ',');
		if(offset)
		{
			memcpy(adval, tokval, offset);
			if (inet_aton(adval, addr))
			{
				addr++;
			}
			tokval += offset + 1;
		}
		else
		{
			if(tokval && strlen(tokval))
			{
				if (inet_aton(tokval, addr))
				{
					addr++;
				}
				break;
			}
			else
				break;
		}
	}
	if(output)
		memcpy(output, address, (num + 1) * 4);
	if(len)
		*len = (num + 1) * 4;
	return 0;
}

static int _dhcpd_option_hex_split(char *input, unsigned char *output, int *len)
{
	unsigned char adval[64];
	unsigned char address[256];
	char *tokval = input;
	int offset = 0, num = 0;
	//struct in_addr *addr = (struct in_addr *)address;
	num = str_chr_count(input, ',');
	if(num == 0)
	{
		char *p = tokval;
		do {
			convert_num(address + offset, p, 16, 8);
			offset += 1;
			p = strstr(p, ":");
			if (p)
				p++;
			else
				break;
		} while (p);

		if(output)
			memcpy(output, address, offset);
		if(len)
			*len = offset;
		return 0;
	}
	num = 0;
	while(offset < strlen(input))
	{
		offset = str_chr_next(tokval, ',');
		if(offset)
		{
			memcpy(adval, tokval, offset);

			char *p = adval;
			do {
				convert_num(address + num, p, 16, 8);
				num += 1;
				p = strstr(p, ":");
				if (p)
					p++;
				else
					break;
			} while (p);

			tokval += offset + 1;
		}
		else
		{
			if(tokval && strlen(tokval))
			{
				char *p = adval;
				do {
					convert_num(address + num, p, 16, 8);
					num += 1;
					p = strstr(p, ":");
					if (p)
						p++;
					else
						break;
				} while (p);
			}
			else
				break;
		}
	}
	if(output)
		memcpy(output, address, num);
	if(len)
		*len = num;
	return 0;
}

static int _dhcpd_option_num_split(int code, char *input, unsigned char *output, int *len)
{
	unsigned char adval[64];
	unsigned char buf[256];
	char *tokval = input;
	int offset = 0, num = 0;
	//struct in_addr *addr = (struct in_addr *)buf;
	num = str_chr_count(input, ',');
	if(num == 0)
	{
		switch(code)
		{
			case 'L': /* Unsigned 32-bit integer. */
			case 'l': /* Signed 32-bit integer. */
				if(output)
					convert_num(output, tokval, 0, 32);
				if(len)
					*len = 4;
				break;
			case 's': /* Signed 16-bit integer. */
			case 'S': /* Unsigned 16-bit integer. */
				if(output)
					convert_num(output, tokval, 0, 16);
				if(len)
					*len = 2;
				break;
			case 'b': /* Signed 8-bit integer. */
			case 'B': /* Unsigned 8-bit integer. */
				if(output)
					convert_num(output, tokval, 0, 8);
				if(len)
					*len = 1;
				break;
			case 'f': /* Boolean flag. */
				if (!strcasecmp(tokval, "true") || !strcasecmp(tokval, "on"))
					if(output)
						output[0] = 1;
				else if (!strcasecmp(tokval, "false") || !strcasecmp(tokval, "off"))
					if(output)
						output[0] = 0;
				else
				{
					parse_warn("expecting boolean.");
					return -1;
				}
				if(len)
					*len = 1;
				break;
			default:
				return -1;
				break;
		}
		return 0;
	}
	num = 0;
	while(offset < strlen(input))
	{
		offset = str_chr_next(tokval, ',');
		if(offset)
		{
			memcpy(adval, tokval, offset);
			switch(code)
			{
				case 'L': /* Unsigned 32-bit integer. */
				case 'l': /* Signed 32-bit integer. */
					convert_num(buf + num, adval, 0, 32);
					num += 4;
					break;
				case 's': /* Signed 16-bit integer. */
				case 'S': /* Unsigned 16-bit integer. */
					convert_num(buf + num, adval, 0, 16);
					num += 2;
					break;
				case 'b': /* Signed 8-bit integer. */
				case 'B': /* Unsigned 8-bit integer. */
					convert_num(buf + num, adval, 0, 8);
					num += 1;
					break;
				case 'f': /* Boolean flag. */
					if (!strcasecmp(adval, "true") || !strcasecmp(adval, "on"))
					{
						buf[num] = 1;
						num += 1;
					}
					else if (!strcasecmp(adval, "false") || !strcasecmp(adval, "off"))
					{
						buf[num] = 0;
						num += 1;
					}
					else
					{
						parse_warn("expecting boolean.");
						return -1;
					}
					break;
				default:
					break;
			}
			tokval += offset + 1;
		}
		else
		{
			if(tokval && strlen(tokval))
			{
				switch(code)
				{
					case 'L': /* Unsigned 32-bit integer. */
					case 'l': /* Signed 32-bit integer. */
						convert_num(buf + num, tokval, 0, 32);
						num += 4;
						break;
					case 's': /* Signed 16-bit integer. */
					case 'S': /* Unsigned 16-bit integer. */
						convert_num(buf + num, tokval, 0, 16);
						num += 2;
						break;
					case 'b': /* Signed 8-bit integer. */
					case 'B': /* Unsigned 8-bit integer. */
						convert_num(buf + num, tokval, 0, 8);
						num++;
						break;
					case 'f': /* Boolean flag. */
						if (!strcasecmp(tokval, "true") || !strcasecmp(tokval, "on"))
						{
							buf[num] = 1;
							num += 1;
						}
						else if (!strcasecmp(tokval, "false") || !strcasecmp(tokval, "off"))
						{
							buf[num] = 0;
							num += 1;
						}
						else
						{
							parse_warn("expecting boolean.");
							return -1;
						}
						break;
					default:
						break;
				}
				break;
			}
			else
				break;
		}
	}
	if(output)
		memcpy(output, buf, num);
	if(len)
		*len = num;
	return 0;
}


static int _dhcpd_option_string_split(char *input, unsigned char *output, int *len)
{
	unsigned char adval[64];
	unsigned char address[256];
	char *tokval = input;
	int offset = 0, num = 0;
	num = str_chr_count(input, ',');
	if(num == 0)
	{
		if(output)
			memcpy(output, tokval, strlen(tokval));
		if(len)
			*len = strlen(tokval);
		return 0;
	}
	while(offset < strlen(input))
	{
		offset = str_chr_next(tokval, ',');
		if(offset)
		{
			memcpy(adval, tokval, offset);
			memcpy(address + num, adval, strlen(adval));
			num += strlen(adval);
			tokval += offset + 1;
		}
		else
		{
			if(tokval && strlen(tokval))
			{
				memcpy(address + num, tokval, strlen(tokval));
				num += strlen(tokval);
				break;
			}
			else
				break;
		}
	}
	if(output)
		memcpy(output, address, num);
	if(len)
		*len = num;
	return 0;
}

static int dhcpd_option_split(int code, char *input, unsigned char *output, int *len)
{
	int ret = -1;
	switch(code)
	{
	case 'I':
		ret = _dhcpd_option_address_split(input, output, len);
		break;
	case 'X':
		ret = _dhcpd_option_hex_split(input, output, len);
		break;
	case 't':
		ret = _dhcpd_option_string_split(input, output, len);
		break;
	case 'C':
		//_dhcpd_option_address_split(input, output, len);
		break;
	default:
		ret = _dhcpd_option_num_split(code, input, output, len);
		break;
	}
	return ret;
}


int dhcpd_option_set(struct group *group, char *optionname,
		char *inval)
{
#ifdef DHCPD_TREE
	int token = 0;
	unsigned char buf[4];
	unsigned char cprefix;
	unsigned char val[256];
	unsigned char adval[64];
	char *tok = NULL;
	char *tokval = inval;
	char *fmt = NULL;
	unsigned int *address = (unsigned int *) buf;
	struct universe *universe = NULL;
	struct dhcpd_option *option = NULL;
	struct tree *tree = NULL;
	struct tree *t = NULL;
	token = TOK_STRING;
	/*
	 * Look up the option name hash table for the specified
	 * vendor.
	 */
	universe = ((struct universe *) dhcp_hash_lookup(&universe_hash,
			(unsigned char *) "dhcp", 0));
	/*
	 * If it's not there, we can't parse the rest of the
	 * declaration.
	 */
	if (!universe)
	{
		parse_warn("no vendor named %s.", "dhcp");
	}
	else
	{
		/*
		 * Use the default hash table, which contains all the
		 * standard dhcp option names.
		 */
		universe = &dhcp_universe;
	}

	/* Look up the actual option info. */
	option = (struct dhcpd_option *) dhcp_hash_lookup(universe->hash,
			(unsigned char *) optionname, 0);

	/* If we didn't get an option structure, it's an undefined option. */
	if (!option)
	{
		parse_warn("no option named %s for vendor %s", val, optionname);
		return -1;
	}
	/* Parse the option data. */
	/*
	 * Set a flag if this is an array of a simple type (i.e.,
	 * not an array of pairs of IP addresses, or something
	 * like that.
	 */
	memset(val, 0, sizeof(val));
	tokval = strstr(inval, ",");
	if (tokval)
	{
		strncpy(val, inval, tokval - inval);
		tok = tokval + 1;
	}
	else
		strcpy(val, inval);
	do
	{
		//int uniform = option->format[1] == 'A';
		for (fmt = option->format; *fmt; fmt++)
		{
			if (*fmt == 'A')
				break;
			switch (*fmt)
			{
			case 'X':
				if(strstr(val, ":"))
				{
					token = TOK_NUMBER_OR_NAME;
				}
				else
					token = TOK_STRING;

				if (token == TOK_NUMBER_OR_NAME)
				{
					char *p = val;
					do {
						convert_num(buf, p, 16, 8);
						tree = tree_concat(tree, tree_const(buf, 1));
						p = strstr(p, ":");
						if (p)
							p++;
						else
							break;
					} while (p);

/*					convert_num(buf, val, 16, 8);
					tree = tree_concat(tree, tree_const(buf, 1));*/
				}
				else if (token == TOK_STRING)
				{
					tree = tree_concat(tree,
							tree_const((unsigned char *) val, strlen(val)));
				}
				break;

			case 't': /* Text string. */
				tree = tree_concat(tree,
						tree_const((unsigned char *) val, strlen(val)));
				break;

			case 'I': /* IP address or hostname. */
				//t = parse_ip_addr_or_hostname(cfile, uniform);
				t = dhcpd_host_addr_tree_lookup(val);
				if (!t)
					return -1;
				tree = tree_concat(tree, t);
				break;

			case 'L': /* Unsigned 32-bit integer. */
			case 'l': /* Signed 32-bit integer. */
				convert_num(buf, val, 0, 32);
				tree = tree_concat(tree, tree_const(buf, 4));
				break;
			case 's': /* Signed 16-bit integer. */
			case 'S': /* Unsigned 16-bit integer. */
				convert_num(buf, val, 0, 16);
				tree = tree_concat(tree, tree_const(buf, 2));
				break;
			case 'b': /* Signed 8-bit integer. */
			case 'B': /* Unsigned 8-bit integer. */
				convert_num(buf, val, 0, 8);
				tree = tree_concat(tree, tree_const(buf, 1));
				break;
			case 'f': /* Boolean flag. */
				if (!strcasecmp(val, "true") || !strcasecmp(val, "on"))
					buf[0] = 1;
				else if (!strcasecmp(val, "false") || !strcasecmp(val, "off"))
					buf[0] = 0;
				else
				{
					parse_warn("expecting boolean.");
					return -1;
				}
				tree = tree_concat(tree, tree_const(buf, 1));
				break;
			case 'C':
				if (strstr(val, "/"))
				{
					char *b = strstr(val, "/");
					if (b)
					{
						memcpy(adval, val, strlen(val) - strlen(b));
						cprefix = atoi(b + 1);
						*address = inet_addr(adval);
						tree = tree_concat(tree,
								tree_const(&cprefix, sizeof(cprefix)));
						if (cprefix > 0)
							tree = tree_concat(tree,
									tree_const(buf, (cprefix + 7) / 8));
					}
				}
				break;
			default:
				dhcpd_warning("Bad format %c in parse_option_param.", *fmt);
				return -1;
			}
		}
		if (strstr(option->format, "A") == NULL)
			break;
		else if (tok)
		{
			tokval = strstr(tok, ",");
			if (tokval)
			{
				memset(val, 0, sizeof(val));
				strncpy(val, tok, tokval - tok);
				tok = tokval + 1;
			}
			else
				break;
		}
		else
			break;
	} while (1);

	group->options[option->code] = tree_cache(tree);
#else
	int token = 0, total_len = 0;
	unsigned char buf[1024];
	char *fmt = NULL;
	struct universe *universe = NULL;
	struct dhcpd_option *option = NULL;
	struct tree_cache *cache = NULL;
	token = TOK_STRING;
	/*
	 * Look up the option name hash table for the specified
	 * vendor.
	 */
	universe = ((struct universe *) dhcp_hash_lookup(&universe_hash,
			(unsigned char *) "dhcp", 0));
	/*
	 * If it's not there, we can't parse the rest of the
	 * declaration.
	 */
	if (!universe)
	{
		parse_warn("no vendor named %s.", "dhcp");
	}
	else
	{
		/*
		 * Use the default hash table, which contains all the
		 * standard dhcp option names.
		 */
		universe = &dhcp_universe;
	}

	/* Look up the actual option info. */
	option = (struct dhcpd_option *) dhcp_hash_lookup(universe->hash,
			(unsigned char *) optionname, 0);

	/* If we didn't get an option structure, it's an undefined option. */
	if (!option)
	{
		parse_warn("no option named %s for vendor %s", inval, optionname);
		return -1;
	}
	/* Parse the option data. */
	/*
	 * Set a flag if this is an array of a simple type (i.e.,
	 * not an array of pairs of IP addresses, or something
	 * like that.
	 */
	memset(buf, 0, sizeof(buf));

	for (fmt = option->format; *fmt; fmt++)
	{
		if (*fmt == 'A')
			break;
		switch (*fmt)
		{
		case 'X':
			if(strstr(inval, ":"))
			{
				token = TOK_NUMBER_OR_NAME;
			}
			else
				token = TOK_STRING;

			if (token == TOK_NUMBER_OR_NAME)
			{
				dhcpd_option_split('X', inval, buf, &total_len);
			}
			else if (token == TOK_STRING)
			{
				dhcpd_option_split('t', inval, buf, &total_len);
			}
			break;

		case 't': /* Text string. */
			dhcpd_option_split('t', inval, buf, &total_len);
			break;

		case 'I': /* IP address or hostname. */
			dhcpd_option_split('I', inval, buf, &total_len);
			break;

		case 'L': /* Unsigned 32-bit integer. */
		case 'l': /* Signed 32-bit integer. */

		case 's': /* Signed 16-bit integer. */
		case 'S': /* Unsigned 16-bit integer. */

		case 'b': /* Signed 8-bit integer. */
		case 'B': /* Unsigned 8-bit integer. */

		case 'f': /* Boolean flag. */
			dhcpd_option_split(*fmt, inval, buf, &total_len);
			break;
		case 'C':
			break;
		default:
			dhcpd_warning("Bad format %c in parse_option_param.", *fmt);
			return -1;
		}
	}

	if(total_len)
	{
		cache = new_tree_cache("server_option_cache");
		if (!cache)
			return -1;
		cache->value = malloc(total_len);
		if(!cache->value)
		{
				free_tree_cache(cache);
				return -1;
		}
		cache->len = cache->buf_size = total_len;
		memcpy(cache->value, buf, cache->len);
		cache->timeout = -1;
		group->options[option->code] = cache;
		int i = 0;
		for(i = 0; i < total_len; i++)
		{
			fprintf(stdout, "0x%02x ",cache->value[i]);
			if((i + 1)/16 == 0)
				fprintf(stdout, "\r\n");
		}
		return 0;
	}
	else
		return -1;
#endif
	return 0;
}

int dhcpd_option55_set(struct group *group, unsigned char *optionkey, int len)
{
	char *fmt = NULL;
	struct universe *universe = NULL;
	struct dhcpd_option *option = NULL;
	/*
	 * Look up the option name hash table for the specified
	 * vendor.
	 */
	universe = ((struct universe *) dhcp_hash_lookup(&universe_hash,
			(unsigned char *) "dhcp", 0));
	/*
	 * If it's not there, we can't parse the rest of the
	 * declaration.
	 */
	if (!universe)
	{
		parse_warn("no vendor named %s.", "dhcp");
	}
	else
	{
		/*
		 * Use the default hash table, which contains all the
		 * standard dhcp option names.
		 */
		universe = &dhcp_universe;
	}

	/* Look up the actual option info. */
	option = (struct dhcpd_option *) dhcp_hash_lookup(universe->hash,
			(unsigned char *) "dhcp-parameter-request-list", 0);

	/* If we didn't get an option structure, it's an undefined option. */
	if (!option)
	{
		parse_warn("no option named %s", "dhcp-parameter-request-list");
		return -1;
	}
	if(group->options[option->code])
	{
		free_tree_cache(group->options[option->code]);
	}
	group->options[option->code] = new_tree_cache("server_option_cache");
	if (!group->options[option->code])
		return -1;

	group->options[option->code]->value = malloc(len);
	if(!group->options[option->code]->value)
	{
			free_tree_cache(group->options[option->code]);
			return -1;
	}
	group->options[option->code]->len = group->options[option->code]->buf_size = len;
	memcpy(group->options[option->code]->value, optionkey, len);
	group->options[option->code]->timeout = -1;
//group->options[option->code] = cache;
	return 0;
}

int dhcpd_option_unset(struct group *group, char *optionname)
{
	char *fmt = NULL;
	struct universe *universe = NULL;
	struct dhcpd_option *option = NULL;
	/*
	 * Look up the option name hash table for the specified
	 * vendor.
	 */
	universe = ((struct universe *) dhcp_hash_lookup(&universe_hash,
			(unsigned char *) "dhcp", 0));
	/*
	 * If it's not there, we can't parse the rest of the
	 * declaration.
	 */
	if (!universe)
	{
		parse_warn("no vendor named %s.", "dhcp");
	}
	else
	{
		/*
		 * Use the default hash table, which contains all the
		 * standard dhcp option names.
		 */
		universe = &dhcp_universe;
	}

	/* Look up the actual option info. */
	option = (struct dhcpd_option *) dhcp_hash_lookup(universe->hash,
			(unsigned char *) optionname, 0);

	/* If we didn't get an option structure, it's an undefined option. */
	if (!option)
	{
		parse_warn("no option named %s", optionname);
		return -1;
	}
	if(group->options[option->code])
	{
		if(group->options[option->code]->value)
			free(group->options[option->code]->value);
		free_tree_cache(group->options[option->code]);
	}
	return 0;
}



int dhcpd_next_server_set(struct group *group, char *hostname_or_address)
{
#ifdef DHCPD_TREE
	//just for relay and client
	struct tree *tree = NULL;
	struct tree_cache *cache = NULL;
	tree = dhcpd_host_addr_tree_lookup(hostname_or_address);
	if (!tree)
		return -1;
	cache = tree_cache(tree);
	if (!tree_evaluate(cache) || !cache)
	{
		dhcpd_error("next-server is not known");
		return -1;
	}
	group->next_server.len = 4;
	memcpy(group->next_server.iabuf, cache->value, group->next_server.len);
#else
	struct in_addr addr;
	if (inet_aton(hostname_or_address, &addr))
	{
		group->next_server.len = 4;
		memcpy(group->next_server.iabuf, &addr.s_addr, group->next_server.len);
	}
	else
	{
		group->next_server.len = strlen(hostname_or_address);
		memcpy(group->next_server.iabuf, hostname_or_address, group->next_server.len);
	}
#endif
	return 0;
}


int dhcpd_server_identifier_set(struct group *group, char *hostname_or_address)
{
#ifdef DHCPD_TREE
	struct tree *tree = NULL;
	struct tree_cache *cache = NULL;
	tree = dhcpd_host_addr_tree_lookup(hostname_or_address);
	if (!tree)
		return -1;
	group->options[DHO_DHCP_SERVER_IDENTIFIER] = tree_cache(tree);
#else
	struct tree_cache *cache = NULL;

	cache = new_tree_cache("server_identifier_cache");
	if (!cache)
		return -1;
	struct in_addr addr;
	if (inet_aton(hostname_or_address, &addr))
	{
		cache->value = malloc(4);
		if(!cache->value)
		{
			free_tree_cache(cache);
			return -1;
		}
		cache->len = cache->buf_size = 4;
		memcpy(cache->value, &addr.s_addr, cache->len);
	}
	else
	{
		cache->value = strdup(hostname_or_address);
		cache->len = cache->buf_size = strlen(hostname_or_address);
	}
	cache->timeout = -1;
	group->options[DHO_DHCP_SERVER_IDENTIFIER] = cache;
#endif
	return 0;
}


int dhcpd_filename_set(struct group *group, char *value)
{
	if (value)
	{
		free(group->filename);
		group->filename = strdup(value);
	}
	else
	{
		if (group->filename)
			free(group->filename);
		group->filename = NULL;
	}
	return 0;
}

int dhcpd_server_name_set(struct group *group, char *value)
{
	if (value)
	{
		free(group->server_name);
		group->server_name = strdup(value);
	}
	else
	{
		if (group->server_name)
			free(group->server_name);
		group->server_name = NULL;
	}
	return 0;
}


int dhcpd_allow_deny_set(struct group *group, int cmd, int val)
{
	switch (cmd)
	{
	case TOK_BOOTP:
		group->allow_bootp = val;
		break;

	case TOK_BOOTING:
		group->allow_booting = val;
		break;

	case TOK_DYNAMIC_BOOTP:
		group->dynamic_bootp = val;
		break;

	case TOK_UNKNOWN_CLIENTS:
		group->boot_unknown_clients = val;
		break;

	case TOK_DYNAMIC_BOOTP_LEASE_CUTOFF:
		group->bootp_lease_cutoff = val;
		break;

	case TOK_GET_LEASE_HOSTNAMES:
		group->get_lease_hostnames = val;
		break;

	case TOK_ALWAYS_REPLY_RFC1048:
		group->always_reply_rfc1048 = val;
		break;

	case TOK_USE_HOST_DECL_NAMES:
		group->use_host_decl_names = val;
		break;

	case TOK_USE_LEASE_ADDR_FOR_DEFAULT_ROUTE:
		group->use_lease_addr_for_default_route = val;
		break;

	default:
		parse_warn("expecting group key");
		return -1;
	}
	return 0;
}

int dhcpd_authoritative_set(struct group *group, int val)
{
	group->authoritative = val;
	return 0;
}


int dhcpd_default_lease_set(struct group *group, int val)
{
	group->default_lease_time = val;
	return 0;
}

int dhcpd_max_lease_set(struct group *group, int val)
{
	group->max_lease_time = val;
	return 0;
}

int dhcpd_bootp_lease_set(struct group *group, int val)
{
	group->bootp_lease_length = val;
	return 0;
}


struct class * dhcpd_class_decl_create(struct group *group, int type, char *val)
{
	struct class *class = NULL;
	class = add_class(type, val);
	if (!class)
	{
		dhcpd_error("No memory for class %s.", val);
		return NULL;
	}
	class->group = clone_group(group, "parse_class_declaration");
	return class;
}

struct shared_network * dhcpd_shared_network_decl_create(struct group *group,
		char *name)
{
	char *n = NULL;
	struct shared_network *share = NULL;
	//int declaration = 0;

	share = calloc(1, sizeof(struct shared_network));
	if (!share)
	{
		dhcpd_error("No memory for shared subnet");
		return NULL;
	}
	share->leases = NULL;
	share->last_lease = NULL;
	share->insertion_point = NULL;
	share->next = NULL;
	share->interface = NULL;
	share->group = clone_group(group, "parse_shared_net_declaration");
	share->group->shared_network = share;
	/* Get the name of the shared network. */
	share->name = strdup(name);
	enter_shared_network(share);
	return share;
}

int dhcpd_shared_network_decl_destroy(char *name)
{
	char *n = NULL;
	struct shared_network *share = NULL;
	//int declaration = 0;
	share = find_shared_network(name);
	if (!share)
	{
		dhcpd_error("No memory for shared subnet");
		return -1;
	}
	/*	share->leases = NULL;
	 share->last_lease = NULL;
	 share->insertion_point = NULL;
	 share->next = NULL;
	 share->interface = NULL;*/
	if (share->group)
	{
		free(share->group);
	}
	//share->group->shared_network = share;
	/* Get the name of the shared network. */
	if (share->name)
		free(share->name);
	//enter_shared_network(share);
	return 0;
}

/*
 subnet 24.25.29.0 netmask 255.255.255.0 {
 range 24.25.29.10 24.25.29.20;
 option broadcast-address 204.254.239.31;
 option routers prelude.fugue.com;
 }
 shared-network -> name = 24.25.29.0
 */
struct subnet *dhcpd_subnet_decl_create(struct shared_network *share,
		struct iaddr *net, struct iaddr *netmask)
{
	struct subnet *subnet = NULL, *t = NULL, *u = NULL;
	struct shared_network *share_network = NULL;
	subnet = calloc(1, sizeof(struct subnet));
	if (!subnet)
	{
		dhcpd_error("No memory for new subnet");
		return NULL;
	}
	if (share == NULL)
	{
		char share_name[64];
		memset(share_name, 0, sizeof(share_name));
		snprintf(share_name, sizeof(share_name), "%d.%d.%d.%d", net->iabuf[0],
				net->iabuf[1], net->iabuf[2], net->iabuf[3]);
		share_network = dhcpd_shared_network_decl_create(&root_group,
				share_name);
	}
	else
		share_network = share;
	if (!share_network)
	{
		free(subnet);
		return NULL;
	}
	subnet->shared_network = share_network;
	subnet->group = clone_group(share_network->group,
			"parse_subnet_declaration");
	subnet->group->subnet = subnet;

	/* Get the network number. */
	memcpy(&subnet->net, net, sizeof(struct iaddr));
	/* Get the netmask. */
	memcpy(&subnet->netmask, netmask, sizeof(struct iaddr));
	/* Save only the subnet number. */
	subnet->net = subnet_number(subnet->net, subnet->netmask);

	enter_subnet(subnet);

	/*
	 * If this subnet supports dynamic bootp, flag it so in the
	 * shared_network containing it.
	 */
	if (subnet->group->dynamic_bootp)
		share_network->group->dynamic_bootp = 1;

	/* Add the subnet to the list of subnets in this shared net. */
	if (!share_network->subnets)
		share_network->subnets = subnet;
	else
	{
		u = NULL;
		for (t = share_network->subnets; t; t = t->next_sibling)
		{
			if (subnet_inner_than(subnet, t, 0))
			{
				if (u)
					u->next_sibling = subnet;
				else
					share_network->subnets = subnet;
				subnet->next_sibling = t;
				return subnet;
			}
			u = t;
		}
		u->next_sibling = subnet;
	}
	return subnet;
}

int dhcpd_subnet_address_range(struct subnet *subnet, struct iaddr *start,
		struct iaddr *stop, int dynamic)
{
	struct iaddr low, high;
	if (1 == dynamic)
	{
		subnet->group->dynamic_bootp = 1;
	}
	/* Get the bottom address in the range. */
	memcpy(&low, start, sizeof(struct iaddr));
	/* Get the top address in the range. */
	memcpy(&high, stop, sizeof(struct iaddr));

	/* Create the new address range. */
	new_address_range(low, high, subnet, dynamic);
	return 0;
}

struct host_decl * dhcpd_host_decl_create(struct group *group, char *name,
		unsigned char *mac, struct iaddr *address, int num)
{
	//char *val;
	//int token;
	struct host_decl *host;
	//char *name = parse_host_name(cfile);
	//int declaration = 0;

	if (!name)
		return NULL;

	host = calloc(1, sizeof(struct host_decl));
	if (!host)
	{
		dhcpd_error("can't allocate host decl struct %s.", name);
		return NULL;
	}
	host->name = strdup(name);
	host->group = clone_group(group, "parse_host_declaration");
	//TODO parse MAC, FIXIP...
#ifdef DHCPD_TREE
	host->fixed_addr = dhcpd_fixed_addr_tree_cache_lookup(address, num);
#endif
	//host->interface = hardware;
	host->interface.htype = HTYPE_ETHER;
	host->interface.hlen = 6;
	memcpy((unsigned char *) &host->interface.haddr[0], mac, 6);

	if (!host->group->options[DHO_HOST_NAME]
			&& host->group->use_host_decl_names)
	{
		host->group->options[DHO_HOST_NAME] = new_tree_cache(
				"parse_host_declaration");
		if (!host->group->options[DHO_HOST_NAME])
			dhcpd_error("can't allocate a tree cache for hostname.");
		host->group->options[DHO_HOST_NAME]->len = strlen(host->name);
		host->group->options[DHO_HOST_NAME]->value =
				(unsigned char *) host->name;
		host->group->options[DHO_HOST_NAME]->buf_size =
				host->group->options[DHO_HOST_NAME]->len;
		host->group->options[DHO_HOST_NAME]->timeout = -1;
#ifdef DHCPD_TREE
		host->group->options[DHO_HOST_NAME]->tree = NULL;
#endif
	}
	dhcpd_debug("%s: add hostname=%s under group=%p\r\n", __func__, name,
			group);
	enter_host(host);
	return 0;
}

int dhcpd_host_decl_destroy(struct group *group, unsigned char *mac, int len)
{
	struct host_decl * host = find_hosts_by_haddr(HTYPE_ETHER, mac, len);
	if (host)
	{
		if (host->group->options[DHO_HOST_NAME])
			free_tree_cache(host->group->options[DHO_HOST_NAME]);

		if (host->name)
			free(host->name);
		if (host->group)
			free(host->group);

		if (host->fixed_addr)	//free tree
			free(host->fixed_addr);
		return 0;
	}
	return -1;
}

static int dhcpd_interface_refresh_one(struct interface_info * iface)
{
	struct iaddr addr;
	struct subnet *subnet = NULL;
	struct shared_network *share = NULL;
	if (!iface)
		return -1;
	if (iface->primary_address.s_addr == 0)
		return 0;
	/* Grab the address... */
	memset(&addr, 0, sizeof(addr));
	addr.len = 4;
	memcpy(addr.iabuf, &iface->primary_address.s_addr, addr.len);

	zlog_debug(6, "interface address : %d.%d.%d.%d",
			addr.iabuf[0],addr.iabuf[1],addr.iabuf[2],addr.iabuf[3]);
	/* If there's a registered subnet for this address,
	 connect it together... */
	if ((subnet = find_subnet(addr)))
	{
		/* If this interface has multiple aliases
		 on the same subnet, ignore all but the
		 first we encounter. */
		if (!subnet->interface)
		{
			subnet->interface = iface;
			subnet->interface_address = addr;
		}
		else if (subnet->interface != iface)
		{
			dhcpd_warning("Multiple %s %s: %s %s", "interfaces match the",
					"same subnet", subnet->interface->name, iface->name);
		}
		share = subnet->shared_network;
		if (iface->shared_network && iface->shared_network != share)
		{
			dhcpd_warning("Interface %s matches %s", iface->name,
					"multiple shared networks");
		}
		else
		{
			iface->shared_network = share;
		}

		if (!share->interface)
		{
			share->interface = iface;
		}
		else if (share->interface != iface)
		{
			dhcpd_warning("Multiple %s %s: %s %s", "interfaces match the",
					"same shared network", share->interface->name, iface->name);
		}
		if (!iface->shared_network)
		{
			dhcpd_warning("Can't listen on %s - dhcpd.conf has no subnet "
					"declaration for %s.", iface->name,
					inet_ntoa(iface->primary_address));
		}
		/* Find subnets that don't have valid interface addresses. */
		for (subnet = (
				iface->shared_network ? iface->shared_network->subnets : NULL);
				subnet; subnet = subnet->next_sibling)
		{
			if (!subnet->interface_address.len)
			{
				/*
				 * Set the interface address for this subnet
				 * to the first address we found.
				 */
				subnet->interface_address.len = 4;
				memcpy(subnet->interface_address.iabuf,
						&iface->primary_address.s_addr, 4);
			}
		}
	}
	return 0;
}

int dhcpd_interface_refresh(void)
{
	struct interface_info *ifp = NULL;
	if (!dhcpd_interfaces_list)
		return NULL;
	for (ifp = dhcpd_interfaces_list; ifp != NULL; ifp = ifp->next)
	{
		if (ifp)
			dhcpd_interface_refresh_one(ifp);
	}
	return 0;
}

static int dhcpd_interface_init(struct interface_info * iface)
{
	struct iaddr addr;
	struct prefix address;
	struct interface * ifp = NULL;

	if (!iface)
		return -1;

	ifp = if_lookup_by_kernel_index(iface->kifindex);
	strcpy(iface->name, ifkernelindex2kernelifname(iface->kifindex));
	iface->ifp = ifp;

	if (nsm_interface_mac_get_api(ifp, iface->hw_address.haddr, 6) == OK)
	{
		iface->hw_address.htype = HTYPE_ETHER;
		iface->hw_address.hlen = 6;
	}

	if (nsm_interface_address_get_api(ifp, &address) == OK)
	{
		iface->primary_address = address.u.prefix4;
	}

	dhcpd_interface_refresh_one(iface);

	return 0;
}

static int dhcpd_interface_exit(struct interface_info * iface)
{
	if (!iface)
		return -1;
	memset(&iface->primary_address, 0, sizeof(struct in_addr));
	memset(&iface->hw_address, 0, sizeof(struct hardware));
	memset(iface->name, 0, sizeof(iface->name));
	iface->ifp = NULL;
	iface->shared_network = NULL;
	iface->rbuf = NULL; /* Read buffer, if required. */
	iface->client = NULL;
	free(iface);
	return 0;
}

struct interface_info * dhcpd_interface_lookup(int ifindex)
{
	struct interface_info *ifp = NULL;
	if (!dhcpd_interfaces_list)
		return NULL;
	for (ifp = dhcpd_interfaces_list; ifp != NULL; ifp = ifp->next)
	{
		if (ifindex && ifp->kifindex == ifindex)
			return ifp;
	}
	return NULL;
}

int dhcpd_interface_add(int ifindex)
{
	struct interface_info *ifp = NULL;
	struct interface_info *next = NULL;
	for (next = dhcpd_interfaces_list; next != NULL; next = next->next)
	{
		if (next->kifindex == ifindex)
		{
			return 1;
		}
	}
	ifp = malloc(sizeof(struct interface_info));
	if (!ifp)
		return -1;
	memset(ifp, 0, sizeof(struct interface_info));
	ifp->kifindex = ifindex;
	dhcpd_interface_init(ifp);
	ifp->next = dhcpd_interfaces_list;
	dhcpd_interfaces_list = ifp;
	return 0;
}

int dhcpd_interface_del(int ifindex)
{
	struct interface_info *pTem = NULL;
	struct interface_info *ifp = dhcpd_interface_lookup(ifindex);
	if (!ifp)
		return -1;
	pTem = NULL;
	if (dhcpd_interfaces_list == ifp)
	{
		dhcpd_interfaces_list = ifp->next;
	}
	else
	{
		for (pTem = dhcpd_interfaces_list; pTem != NULL; pTem = pTem->next)
		{
			if (pTem->next == ifp)
			{
				break;
			}
		}
		if (!pTem)
		{
			dhcpd_interface_exit(ifp);
			return 0;
		}
		pTem->next = ifp->next;
	}
	dhcpd_interface_exit(ifp);
	return 0;
}
