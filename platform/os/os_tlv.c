/*
 * os_tlv.c
 *
 *  Created on: 2018��12��19��
 *      Author: DELL
 */

#include <string.h>
#include <malloc.h>

#include "os_tlv.h"


int os_tlv_set_string(char *input, tag_t tag, len_t len, void * val)
{
	os_tlv_t *tlv = (os_tlv_t *)input;
	tlv->tag = htonl(tag);
	tlv->len = htonl(len);
	memcpy(tlv->val.pval, val, strlen(val));
	return (sizeof(tag_t) + sizeof(len_t) + strlen(val));
}


int os_tlv_set_integer(char *input, tag_t tag, len_t len, void * val)
{
	os_tlv_t *tlv = (os_tlv_t *)input;
	tlv->tag = htonl(tag);
	tlv->len = htonl(len);
	switch(len)
	{
	case 1:
		tlv->val.val8 = (val_t*)val;
		break;
	case 2:
		{
			unsigned short *inpv = (unsigned short *)val;
			tlv->val.val16 = htons(*inpv);
		}
		break;
	case 4:
		{
			unsigned int *inpv = (unsigned int *)val;
			tlv->val.val32 = htonl(*inpv);
		}
		break;
	default:
		{
			//tlv->len = htonl(len);
			memcpy(tlv->val.pval, val, strlen(val));
		}
		break;
	}
	return (sizeof(tag_t) + sizeof(len_t) + len);
}

int os_tlv_set_zero(char *input, tag_t tag, len_t len)
{
	os_tlv_t *tlv = (os_tlv_t *)input;
	tlv->tag = htonl(tag);
	tlv->len = htonl(len);
	return (sizeof(tag_t) + sizeof(len_t));
}


int os_tlv_get(char *input, os_tlv_t *tlv)
{
	os_tlv_t *itlv = (os_tlv_t *)input;
	tlv->tag = ntohl(itlv->tag);
	tlv->len = ntohl(itlv->len);
	switch(tlv->len)
	{
	case 1:
		tlv->val.val8 = itlv->val.val8;
		tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		break;
	case 2:
		{
			tlv->val.val16 = htons(itlv->val.val16);
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	case 4:
		{
			tlv->val.val32 = htonl(itlv->val.val32);
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	default:
		{
			//memcpy(tlv->val.pval, itlv->val.pval, tlv->len);
			//tlv->val.pval = itlv->val.pval;
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	}
	//tlv->val = itlv->val;
	return (sizeof(tag_t) + sizeof(len_t) + tlv->len);
}

int os_tlv_value_get(char *input, os_tlv_t *tlv)
{
	os_tlv_t *itlv = (os_tlv_t *)input;
	tlv->tag = ntohl(itlv->tag);
	tlv->len = ntohl(itlv->len);
	switch(tlv->len)
	{
	case 1:
		tlv->val.val8 = itlv->val.val8;
		tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		break;
	case 2:
		{
			tlv->val.val16 = htons(itlv->val.val16);
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	case 4:
		{
			tlv->val.val32 = htonl(itlv->val.val32);
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	default:
		{
			//memcpy(tlv->val.pval, itlv->val.pval, tlv->len);
			//tlv->val.pval = itlv->val.pval;
			tlv->val.pval = (unsigned char *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	}
	//tlv->val = itlv->val;
	return (sizeof(tag_t) + sizeof(len_t) + tlv->len);
}
