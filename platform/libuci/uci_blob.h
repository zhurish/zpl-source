/*
 * blob.c - uci <-> blobmsg conversion layer
 * Copyright (C) 2012-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */
#ifndef __UCI_BLOB_H
#define __UCI_BLOB_H

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifdef LIBUBOX_ENABLE

#define bool int

struct blob_attr {
	int  id_len;
	char data[];
} __packed;

struct blob_attr_info {
	unsigned int type;
	unsigned int minlen;
	unsigned int maxlen;
	bool (*validate)(const struct blob_attr_info *, struct blob_attr *);
};

struct blob_buf {
	struct blob_attr *head;
	bool (*grow)(struct blob_buf *buf, int minlen);
	int buflen;
	void *buf;
};
#include <libubox/blobmsg.h>
#define BLOBMSG_ALIGN	2
#define BLOBMSG_PADDING(len) (((len) + (1 << BLOBMSG_ALIGN) - 1) & ~((1 << BLOBMSG_ALIGN) - 1))

enum blobmsg_type {
	BLOBMSG_TYPE_UNSPEC,
	BLOBMSG_TYPE_ARRAY,
	BLOBMSG_TYPE_TABLE,
	BLOBMSG_TYPE_STRING,
	BLOBMSG_TYPE_INT64,
	BLOBMSG_TYPE_INT32,
	BLOBMSG_TYPE_INT16,
	BLOBMSG_TYPE_INT8,
	BLOBMSG_TYPE_DOUBLE,
	__BLOBMSG_TYPE_LAST,
	BLOBMSG_TYPE_LAST = __BLOBMSG_TYPE_LAST - 1,
	BLOBMSG_TYPE_BOOL = BLOBMSG_TYPE_INT8,
};

struct blobmsg_hdr {
	unsigned short namelen;
	unsigned char name[];
} __packed;

struct blobmsg_policy {
	const char *name;
	enum blobmsg_type type;
};
#include "uci.h"

struct uci_blob_param_info {
	enum blobmsg_type type;
};

struct uci_blob_param_list {
	int n_params;
	const struct blobmsg_policy *params;
	const struct uci_blob_param_info *info;
	const char * const *validate;

	int n_next;
	const struct uci_blob_param_list *next[];
};

int uci_to_blob(struct blob_buf *b, struct uci_section *s,
		const struct uci_blob_param_list *p);
bool uci_blob_check_equal(struct blob_attr *c1, struct blob_attr *c2,
			  const struct uci_blob_param_list *config);
bool uci_blob_diff(struct blob_attr **tb1, struct blob_attr **tb2,
		   const struct uci_blob_param_list *config,
		   unsigned long *diff_bits);

#endif
#endif
