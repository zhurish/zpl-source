/*	$OpenBSD: dhcpd.h,v 1.55 2016/10/06 16:12:43 krw Exp $ */

/*
 * Copyright (c) 1995, 1996, 1997, 1998, 1999
 * The Internet Software Consortium.    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */
#ifndef __DHCPD_H__
#define __DHCPD_H__

#include <netinet/in.h>

#include "config.h"
//#define ifr_netmask ifr_addr

//#define DHCPD_ANSYNC_ENABLE
//#define DHCPD_CONF_DEBUG
/*
#define strlcpy	strncpy
#define strlcat strncat
#define syslog_r syslog
*/
//#define parse_warn printf
//#define DHCPD_TREE

//#define __dead

//#define HAVE_SA_LEN
#define HAVE_MKSTEMP

#define DB_TIMEFMT	"%w %Y/%m/%d %T UTC"
#define OLD_DB_TIMEFMT	"%w %Y/%m/%d %T"

#define SERVER_PORT	67
#define CLIENT_PORT	68

struct iaddr {
	int len;
	unsigned char iabuf[16];
};

struct iaddrlist {
	struct iaddrlist *next;
	struct iaddr addr;
};

#define DEFAULT_HASH_SIZE	97

struct hash_bucket {
	struct hash_bucket *next;
	unsigned char *name;
	int len;
	unsigned char *value;
};

struct hash_table {
	int hash_count;
	struct hash_bucket *buckets[DEFAULT_HASH_SIZE];
};

struct option_data {
	int len;
	u_int8_t *data;
};

struct string_list {
	struct string_list *next;
	char *string;
};

/* A name server, from /etc/resolv.conf. */
struct name_server {
	struct name_server *next;
	struct sockaddr_in addr;
	time_t rcdate;
};

/* A domain search list element. */
struct domain_search_list {
	struct domain_search_list *next;
	char *domain;
	time_t rcdate;
};

/* A dhcp packet and the pointers to its option values. */
struct packet {
	struct dhcp_packet *raw;
	int packet_length;
	int packet_type;
	int options_valid;
	int client_port;
	struct iaddr client_addr;
	struct interface_info *interface;	/* Interface on which packet
						   was received. */
	struct hardware *haddr;		/* Physical link address
					   of local sender (maybe gateway). */
	struct shared_network *shared_network;
	struct option_data options[256];
	int got_requested_address;	/* True if client sent the
					   dhcp-requested-address option. */
};

struct hardware {
	u_int8_t htype;
	u_int8_t hlen;
	u_int8_t haddr[16];
};

/* A dhcp lease declaration structure. */
struct lease {
	struct lease *next;
	struct lease *prev;
	struct lease *n_uid, *n_hw;
	struct lease *waitq_next;

	struct iaddr ip_addr;
	time_t starts, ends, timestamp;
	unsigned char *uid;
	int uid_len;
	int uid_max;
	unsigned char uid_buf[32];
	char *hostname;
	char *client_hostname;
	uint8_t *client_identifier;
	struct host_decl *host;
	struct subnet *subnet;
	struct shared_network *shared_network;
	struct hardware hardware_addr;

	int client_identifier_len;
	int flags;
#define STATIC_LEASE		1
#define BOOTP_LEASE		2
#define DYNAMIC_BOOTP_OK	4
#define PERSISTENT_FLAGS	(DYNAMIC_BOOTP_OK)
#define EPHEMERAL_FLAGS		(BOOTP_LEASE)
#define MS_NULL_TERMINATION	8
#define ABANDONED_LEASE		16
#define INFORM_NOLEASE		32

	struct lease_state *state;
	u_int8_t releasing;
};

struct lease_state {
	struct lease_state *next;

	struct interface_info *ip;

	time_t offered_expiry;

	struct tree_cache *options[256];
	u_int32_t expiry, renewal, rebind;
	char filename[DHCP_FILE_LEN];
	char *server_name;

	struct iaddr from;

	int max_message_size;
	u_int8_t *prl;
	int prl_len;
	int got_requested_address;	/* True if client sent the
					   dhcp-requested-address option. */
	int got_server_identifier;	/* True if client sent the
					   dhcp-server-identifier option. */
	struct shared_network *shared_network;	/* Shared network of interface
						   on which request arrived. */

	u_int32_t xid;
	u_int16_t secs;
	u_int16_t bootp_flags;
	struct in_addr ciaddr;
	struct in_addr giaddr;
	u_int8_t hops;
	u_int8_t offer;
	struct hardware haddr;
};

#define	ROOT_GROUP	0
#define HOST_DECL	1
#define SHARED_NET_DECL	2
#define SUBNET_DECL	3
#define CLASS_DECL	4
#define	GROUP_DECL	5

/* Group of declarations that share common parameters. */
struct group {
	struct group *next;

	struct subnet *subnet;
	struct shared_network *shared_network;

	time_t default_lease_time;
	time_t max_lease_time;
	time_t bootp_lease_cutoff;
	time_t bootp_lease_length;

	char *filename;
	char *server_name;
	struct iaddr next_server;

	int boot_unknown_clients;
	int dynamic_bootp;
	int allow_bootp;
	int allow_booting;
	int get_lease_hostnames;
	int use_host_decl_names;
	int use_lease_addr_for_default_route;
	int authoritative;
	int always_reply_rfc1048;

	struct tree_cache *options[256];

	u_int16_t server_port;
	u_int16_t client_port;
	time_t cur_time;
	time_t last_scan;
};

/* A dhcp host declaration structure. */
struct host_decl {
	struct host_decl *n_ipaddr;
	char *name;
	struct hardware interface;
	struct tree_cache *fixed_addr;
	struct group *group;
};

struct shared_network {
	struct shared_network *next;
	char *name;
	struct subnet *subnets;
	struct interface_info *interface;
	struct lease *leases;
	struct lease *insertion_point;
	struct lease *last_lease;

	struct group *group;
};

struct subnet {
	struct subnet *next_subnet;
	struct subnet *next_sibling;
	struct shared_network *shared_network;
	struct interface_info *interface;
	struct iaddr interface_address;
	struct iaddr net;
	struct iaddr netmask;

	struct group *group;
};

struct class {
	char *name;

	struct group *group;
};

/* DHCP client lease structure... */
struct client_lease {
	struct client_lease *next;		/* Next lease in list. */
	time_t expiry, renewal, rebind;		/* Lease timeouts. */
	struct iaddr address;			/* Address being leased. */
	char *server_name;			/* Name of boot server. */
	char *filename;				/* File to boot. */
	struct string_list *medium;		/* Network medium. */

	unsigned int is_static : 1;	/* If set, lease is from config file. */
	unsigned int is_bootp: 1;	/* If set, lease was aquired with BOOTP. */

	struct option_data options[256];	/* Options supplied with lease. */
};

/* privsep message. fixed length for easy parsing */
struct pf_cmd {
	struct in_addr ip;
	u_int32_t type;
};

/* Possible states in which the client can be. */
enum dhcp_state {
	S_REBOOTING,
	S_INIT,
	S_SELECTING,
	S_REQUESTING,
	S_BOUND,
	S_RENEWING,
	S_REBINDING
};

/* Configuration information from the config file... */
struct client_config {
	struct option_data defaults[256]; /* Default values for options. */
	enum {
		ACTION_DEFAULT,		/* Use server value if present,
					   otherwise default. */
		ACTION_SUPERSEDE,	/* Always use default. */
		ACTION_PREPEND,		/* Prepend default to server. */
		ACTION_APPEND		/* Append default to server. */
	} default_actions[256];

	struct option_data send_options[256]; /* Send these to server. */
	u_int8_t required_options[256]; /* Options server must supply. */
	u_int8_t requested_options[256]; /* Options to request from server. */
	int requested_option_count;	/* Number of requested options. */
	time_t timeout;			/* Start to panic if we don't get a
					   lease in this time period when
					   SELECTING. */
	time_t initial_interval;	/* All exponential backoff intervals
					   start here. */
	time_t retry_interval;		/* If the protocol failed to produce
					   an address before the timeout,
					   try the protocol again after this
					   many seconds. */
	time_t select_interval;		/* Wait this many seconds from the
					   first DHCPDISCOVER before
					   picking an offered lease. */
	time_t reboot_timeout;		/* When in INIT-REBOOT, wait this
					   long before giving up and going
					   to INIT. */
	time_t backoff_cutoff;		/* When doing exponential backoff,
					   never back off to an interval
					   longer than this amount. */
	struct string_list *media;	/* Possible network media values. */
	char *script_name;		/* Name of config script. */
	enum { IGNORE, ACCEPT, PREFER } bootp_policy;
					/* Ignore, accept or prefer BOOTP
					   responses. */
	struct string_list *medium;	/* Current network medium. */

	struct iaddrlist *reject_list;	/* Servers to reject. */
};

/* Per-interface state used in the dhcp client... */
struct client_state {
	struct client_lease *active;		  /* Currently active lease. */
	struct client_lease *new;			       /* New lease. */
	struct client_lease *offered_leases;	    /* Leases offered to us. */
	struct client_lease *leases;		/* Leases we currently hold. */
	struct client_lease *alias;			     /* Alias lease. */

	enum dhcp_state state;		/* Current state for this interface. */
	struct iaddr destination;		    /* Where to send packet. */
	u_int32_t xid;					  /* Transaction ID. */
	u_int16_t secs;			    /* secs value from DHCPDISCOVER. */
	time_t first_sending;			/* When was first copy sent? */
	time_t interval;	      /* What's the current resend interval? */
	struct string_list *medium;		   /* Last media type tried. */

	struct dhcp_packet packet;		    /* Outgoing DHCP packet. */
	int packet_length;	       /* Actual length of generated packet. */

	struct iaddr requested_address;	    /* Address we would like to get. */

	struct client_config *config;	    /* Information from config file. */

	char **scriptEnv;  /* Client script env */
	int scriptEnvsize; /* size of the env table */

	struct string_list *env;	       /* Client script environment. */
	int envc;			/* Number of entries in environment. */
};

/* Information about each network interface. */

struct interface_info {
	struct interface_info *next;	/* Next interface in list... */
	struct shared_network *shared_network;
				/* Networks connected to this interface. */
	struct hardware hw_address;	/* Its physical address. */
	struct in_addr primary_address;	/* Primary interface address. */
	char name[IFNAMSIZ];		/* Its name... */
	int kifindex;
	unsigned char relay_address;
	//int rfdesc;			/* Its read file descriptor. */
	int wfdesc;			/* Its write file descriptor, if
					   different. */
	unsigned char *rbuf;		/* Read buffer, if required. */
	size_t rbuf_max;		/* Size of read buffer. */
	size_t rbuf_offset;		/* Current offset into buffer. */
	size_t rbuf_len;		/* Length of data in buffer. */

	//struct ifreq *ifp;		/* Pointer to ifreq struct. */
	void *ifp;
	/* Only used by DHCP client code. */
	struct client_state *client;
	int noifmedia;
	int errors;
	int dead;
	//u_int16_t	index;
	int is_udpsock;
	ssize_t (*send_packet)(struct interface_info *, struct dhcp_packet *,
	    size_t, struct in_addr, struct sockaddr_in *, struct hardware *);
};

struct hardware_link {
	struct hardware_link *next;
	char name[IFNAMSIZ];
	struct hardware address;
};

#ifndef DHCPD_ANSYNC_ENABLE
struct dhcpd_timeout {
	struct dhcpd_timeout *next;
	time_t when;
	void (*func)(void *);
	void *what;
};

struct protocol {
	struct protocol *next;
	int fd;
	void (*handler)(struct protocol *);
	void *local;
	char name[16];
};
#endif
/* Bitmask of dhcp option codes. */
typedef unsigned char option_mask[16];

/* DHCP Option mask manipulation macros... */
#define OPTION_ZERO(mask)	(memset (mask, 0, 16))
#define OPTION_SET(mask, bit)	(mask[bit >> 8] |= (1 << (bit & 7)))
#define OPTION_CLR(mask, bit)	(mask[bit >> 8] &= ~(1 << (bit & 7)))
#define OPTION_ISSET(mask, bit)	(mask[bit >> 8] & (1 << (bit & 7)))
#define OPTION_ISCLR(mask, bit)	(!OPTION_ISSET (mask, bit))

/* An option occupies its length plus two header bytes (code and
    length) for every 255 bytes that must be stored. */
#define OPTION_SPACE(x)		((x) + 2 * ((x) / 255 + 1))

#define _PATH_DHCPD_CONF	SYSCONFDIR"/dhcpd.conf"
#define _PATH_DHCPD_DB		SYSCONFDIR"/dhcpd.leases"
#define _PATH_DEV_PF		"/dev/pf"
#define DHCPD_LOG_FACILITY	LOG_DAEMON

#define MAX_TIME 0x7fffffff
#define MIN_TIME 0

/* External definitions... */

/* reallocarray.c */
extern void *reallocarray(void *, size_t, size_t);

/* options.c */
extern void	 parse_options(struct packet *);
extern void	 parse_option_buffer(struct packet *, unsigned char *, int);
extern int	 cons_options(struct packet *, struct dhcp_packet *, int,
	    struct tree_cache **, int, int, int, u_int8_t *, int);
extern char	*pretty_print_option(unsigned int, unsigned char *, int, int, int);
extern void	 do_packet(struct interface_info *, struct dhcp_packet *, int,
	    unsigned int, struct iaddr, struct hardware *);

/* errwarn.c */
extern int warnings_occurred;
extern void	dhcpd_error(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern int	dhcpd_warning(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern int	dhcpd_note(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern int	dhcpd_debug(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern int	parse_warn(char *, ...) __attribute__ ((__format__ (__printf__, 1, 2)));

/* dhcpd.c */
//extern time_t		cur_time;
extern struct group	root_group;

//extern u_int16_t	server_port;
//extern u_int16_t	client_port;
//extern int		log_priority;
//extern int		log_perror;

//
extern char		*path_dhcpd_db;

//int	main(int, char *[]);
//extern void	cleanup(void);

extern void	lease_pinged(struct iaddr, u_int8_t *, int);
extern void	lease_ping_timeout(void *);
extern void	periodic_scan(void *);

/* conflex.c */
#ifdef DHCPD_CONF_DEBUG
extern char		*path_dhcpd_conf;
extern int	 lexline, lexchar;
extern char	*token_line, *tlname;
extern char	 comments[4096];
extern int	 comment_index;
extern int	 eol_token;


extern void	new_parse(char *);
extern int	next_token(char **, FILE *);
extern int	peek_token(char **, FILE *);

/* confpars.c */

extern int	 readconf(void);
extern void	 read_leases(void);
extern int	 parse_statement(FILE *, struct group *, int, struct host_decl *, int);
extern void	 parse_allow_deny(FILE *, struct group *, int);
extern void	 skip_to_semi(FILE *);
extern int	 parse_boolean(FILE *);
extern int	 parse_semi(FILE *);
extern int	 parse_lbrace(FILE *);
extern void	 parse_host_declaration(FILE *, struct group *);
extern char	*parse_host_name(FILE *);
extern void	 parse_class_declaration(FILE *, struct group *, int);
extern void	 parse_lease_time(FILE *, time_t *);
extern void	 parse_shared_net_declaration(FILE *, struct group *);
extern void	 parse_subnet_declaration(FILE *, struct shared_network *);
extern void	 parse_group_declaration(FILE *, struct group *);
extern void	 parse_hardware_param(FILE *, struct hardware *);
extern char	*parse_string(FILE *);

extern struct tree		*parse_ip_addr_or_hostname(FILE *, int);
extern struct tree_cache	*parse_fixed_addr_param(FILE *);
extern void			 parse_option_param(FILE *, struct group *);
extern time_t			 parse_timestamp(FILE *);
extern struct lease	 *parse_lease_declaration(FILE *);
extern void			 parse_address_range(FILE *, struct subnet *);
extern time_t			 parse_date(FILE *);
extern unsigned char		*parse_numeric_aggregate(FILE *, unsigned char *, int *,
			    int, int, int);
extern void			 convert_num(unsigned char *, char *, int, int);
#endif

/* tree.c */
#ifdef DHCPD_TREE
extern pair			 cons(caddr_t, pair);
extern struct tree_cache	*tree_cache(struct tree *);
extern struct tree		*tree_host_lookup(char *);
#endif
extern struct dns_host_entry	*enter_dns_host(char *);
#ifdef DHCPD_TREE
extern struct tree		*tree_const(unsigned char *, int);
extern struct tree		*tree_concat(struct tree *, struct tree *);
extern struct tree		*tree_limit(struct tree *, int);
#endif
extern int			 tree_evaluate(struct tree_cache *);

/* dhcp.c */
extern int	outstanding_pings;

extern void dhcp(struct packet *, int);
extern void dhcpdiscover(struct packet *);
extern void dhcprequest(struct packet *);
extern void dhcprelease(struct packet *);
extern void dhcpdecline(struct packet *);
extern void dhcpinform(struct packet *);
extern void nak_lease(struct packet *, struct iaddr *cip);
extern void ack_lease(struct packet *, struct lease *, unsigned int, time_t);
extern void dhcp_reply(struct lease *, unsigned short);
extern struct lease *find_lease(struct packet *, struct shared_network *, int *);
extern struct lease *mockup_lease(struct packet *, struct shared_network *,
    struct host_decl *);

/* bootp.c */
extern void bootp(struct packet *);

/* memory.c */
extern void enter_host(struct host_decl *);
extern struct host_decl *find_hosts_by_haddr(int, unsigned char *, int);
extern struct host_decl *find_hosts_by_uid(unsigned char *, int);
extern struct subnet *find_host_for_network(struct host_decl **, struct iaddr *,
    struct shared_network *);
extern void new_address_range(struct iaddr, struct iaddr, struct subnet *, int);
extern void free_address_range(struct iaddr , struct iaddr , struct subnet *);

extern struct subnet *find_grouped_subnet(struct shared_network *,
    struct iaddr);
extern struct subnet *find_subnet(struct iaddr);
extern void enter_shared_network(struct shared_network *);
extern struct shared_network *find_shared_network(char *name);
extern int free_shared_network(struct shared_network *share);

extern int subnet_inner_than(struct subnet *, struct subnet *, int);
extern void enter_subnet(struct subnet *);
extern int free_subnet(struct subnet *subnet);
extern void enter_lease(struct lease *);
extern int supersede_lease(struct lease *, struct lease *, int);
extern void release_lease(struct lease *);
extern void abandon_lease(struct lease *, char *);
extern struct lease *find_lease_by_uid(unsigned char *, int);
extern struct lease *find_lease_by_hw_addr(unsigned char *, int);
extern struct lease *find_lease_by_ip_addr(struct iaddr);
extern void uid_hash_add(struct lease *);
extern void uid_hash_delete(struct lease *);
extern void hw_hash_add(struct lease *);
extern void hw_hash_delete(struct lease *);
extern struct class *add_class(int, char *);
extern struct class *find_class(int, unsigned char *, int);
extern int free_class(int , unsigned char *);
extern struct group *clone_group(struct group *, char *);
extern void write_leases(void);

/* alloc.c */
extern struct tree_cache *new_tree_cache(char *);
extern struct lease_state *new_lease_state(char *);
extern void free_lease_state(struct lease_state *, char *);
extern void free_tree_cache(struct tree_cache *);

/* print.c */
char *print_hw_addr(int, int, unsigned char *);

/* bpf.c */
/*
int if_register_bpf(struct interface_info *);
extern void if_register_send(struct interface_info *);
extern void if_register_receive(struct interface_info *);
ssize_t receive_packet(struct interface_info *, unsigned char *, size_t,
    struct sockaddr_in *, struct hardware *);
*/

/* dispatch.c */
extern struct interface_info *dhcpd_interfaces_list;
//extern struct protocol *protocols;
//extern struct dhcpd_timeout *timeouts;
//extern void discover_interfaces(int *);
#ifndef DHCPD_ANSYNC_ENABLE
extern void dispatch(void);
#endif
extern int locate_network(struct packet *);
//extern void got_one(struct protocol *);
#ifndef DHCPD_ANSYNC_ENABLE
extern void add_timeout(time_t, void (*)(void *), void *);
extern void cancel_timeout(void (*)(void *), void *);
extern void add_protocol (char *, int, void (*)(struct protocol *), void *);
extern void remove_protocol(struct protocol *);
#else
#include "os_ansync.h"
extern os_ansync_lst *dhcpd_lstmaster;
extern void dispatch(void);

/*extern void add_timeout(time_t, void (*)(void *), void *);
extern void cancel_timeout(void (*)(void *), void *);
extern void add_protocol (char *, int, void (*)(struct protocol *), void *);
extern void remove_protocol(struct protocol *);*/

#define add_protocol(a, i, f, v) os_ansync_add(dhcpd_lstmaster, OS_ANSYNC_INPUT, f, v, i)
#define remove_protocol(v)		os_ansync_del(dhcpd_lstmaster, OS_ANSYNC_INPUT, NULL, v, 0)

#define add_timeout(i, f, v) 	os_ansync_add(dhcpd_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, i * 1000)
#define cancel_timeout(f, v)	os_ansync_del_all(dhcpd_lstmaster, OS_ANSYNC_TIMER_ONCE, f, v, 0)

#endif
/* hash.c */
extern struct hash_table *new_hash(void);
extern void add_hash(struct hash_table *, unsigned char *, int, unsigned char *);
extern void delete_hash_entry(struct hash_table *, unsigned char *, int);
extern unsigned char *dhcp_hash_lookup(struct hash_table *, unsigned char *, int);

/* tables.c */
extern struct dhcpd_option dhcp_options[256];
extern unsigned char dhcp_option_default_priority_list[256];

extern char *hardware_types[256];
extern struct hash_table universe_hash;
extern struct universe dhcp_universe;
extern void initialize_universes(void);
extern char *dhcpd_option_to_name(int code);
extern int intern(char *atom, int dfv);

/* convert.c */
extern u_int32_t getULong(unsigned char *);
extern int32_t getLong(unsigned char *);
extern u_int16_t getUShort(unsigned char *);
extern int16_t getShort(unsigned char *);
extern void putULong(unsigned char *, u_int32_t);
extern void putLong(unsigned char *, int32_t);
extern void putUShort(unsigned char *, unsigned int);
extern void putShort(unsigned char *, int);

/* inet.c */
extern struct iaddr subnet_number(struct iaddr, struct iaddr);
extern struct iaddr ip_addr(struct iaddr, struct iaddr, u_int32_t);
extern u_int32_t host_addr(struct iaddr, struct iaddr);
extern int addr_eq(struct iaddr, struct iaddr);
extern char *piaddr(struct iaddr);

/* db.c */
extern int write_lease(struct lease *);
extern int commit_leases(void);
extern void db_startup(void);
extern void new_lease_file(void);

/* packet.c */
extern void assemble_hw_header(struct interface_info *, unsigned char *,
    int *, struct hardware *);
extern void assemble_udp_ip_header(struct interface_info *, unsigned char *,
    int *, u_int32_t, u_int32_t, unsigned int, unsigned char *, int);
extern ssize_t decode_hw_header(struct interface_info *, unsigned char *,
    int, struct hardware *);
extern ssize_t decode_udp_ip_header(struct interface_info *, unsigned char *,
    int, struct sockaddr_in *, int);
extern u_int32_t	checksum(unsigned char *, unsigned, u_int32_t);
extern u_int32_t	wrapsum(u_int32_t);

/* icmp.c */
extern void icmp_startup(int, void (*)(struct iaddr, u_int8_t *, int));
extern int icmp_echorequest(struct iaddr *);
extern void icmp_echoreply(struct protocol *);

/* pfutils.c */
/*
extern void pftable_handler(void);
extern void pf_change_table(int, int, struct in_addr, char *);
extern void pf_kill_state(int, struct in_addr);
extern size_t atomicio(ssize_t (*)(int, void *, size_t), int, void *, size_t);
#define vwrite (ssize_t (*)(int, void *, size_t))write
extern void pfmsg(char, struct lease *);
extern struct syslog_data sdata;
*/

/* udpsock.c */
extern void udpsock_startup(struct in_addr);


#include "dhcpd_utils.h"

#endif
