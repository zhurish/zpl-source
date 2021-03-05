/*
 * os_tlv.c
 *
 *  Created on: 2018��12��19��
 *      Author: DELL
 */
#include "zebra.h"
#include <string.h>
#include <malloc.h>

#include "os_tlv.h"


int os_tlv_set_string(char *input, tag_t tag, len_t len, void * val)
{
	if(val && len)
	{
		os_tlv_t *tlv = (os_tlv_t *)input;
		tlv->tag = htonl(tag);
		tlv->len = htonl(len);
		tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		memcpy(tlv->val.pval, (char *)val, MIN(strlen(val), len));
		return (sizeof(tag_t) + sizeof(len_t) + len);
	}
	return 0;
}

int os_tlv_set_octet(char *input, tag_t tag, len_t len, void * val)
{
	if(val && len)
	{
		os_tlv_t *tlv = (os_tlv_t *)input;
		tlv->tag = htonl(tag);
		tlv->len = htonl(len);
		tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		memcpy(tlv->val.pval, (char *)val, len);
		return (sizeof(tag_t) + sizeof(len_t) + len);
	}
	return 0;
}

/*int os_tlv_set_value(char *input, tag_t tag, len_t len, void * val, len_t vlen)
{
	os_tlv_t *tlv = (os_tlv_t *)input;
	tlv->tag = htonl(tag);
	tlv->len = htonl(len);
	memcpy(tlv->val.pval, val, vlen);
	return (sizeof(tag_t) + sizeof(len_t) + vlen);
}*/

int os_tlv_set_integer(char *input, tag_t tag, len_t len, void * val)
{
	os_tlv_t *tlv = (os_tlv_t *)input;
	if(!val || !len)
		return 0;
	tlv->tag = htonl(tag);
	tlv->len = htonl(len);
	//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
	switch(len)
	{
	case 1:
		tlv->val.val8 = *((val_t *)val);
		break;
	case 2:
		{
			ospl_uint16 *inpv = (ospl_uint16 *)val;
			tlv->val.val16 = htons(*inpv);
		}
		break;
	case 4:
		{
			ospl_uint32 *inpv = (ospl_uint32 *)val;
			tlv->val.val32 = htonl(*inpv);
		}
		break;
	default:
		{
			//tlv->len = htonl(len);
			tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
			memcpy(tlv->val.pval, val, len);
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
		//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		tlv->val.val8 = itlv->val.val8;
		break;
	case 2:
		{
			//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
			tlv->val.val16 = htons(itlv->val.val16);
		}
		break;
	case 4:
		{
			//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
			tlv->val.val32 = htonl(itlv->val.val32);
		}
		break;
	default:
		{
			//memcpy(tlv->val.pval, itlv->val.pval, tlv->len);
			//tlv->val.pval = itlv->val.pval;
			tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	}
	tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
	//tlv->val.val8;
	//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
	//tlv->val = itlv->val;
	return (sizeof(tag_t) + sizeof(len_t) + tlv->len);
}

int os_tlv_value_get(char *input, os_tlv_t *tlv, len_t len)
{
	os_tlv_t *itlv = (os_tlv_t *)input;
	tlv->tag = ntohl(itlv->tag);
	tlv->len = ntohl(itlv->len);
	switch(/*tlv->*/len)
	{
	case 1:
		//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		tlv->val.val8 = itlv->val.val8;
		break;
	case 2:
		{
			//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
			tlv->val.val16 = htons(itlv->val.val16);
		}
		break;
	case 4:
		{
			//tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
			tlv->val.val32 = htonl(itlv->val.val32);
		}
		break;
	default:
		{
			//memcpy(tlv->val.pval, itlv->val.pval, tlv->len);
			//tlv->val.pval = itlv->val.pval;
			tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
		}
		break;
	}
	tlv->val.pval = (ospl_uint8 *)(input + sizeof(tag_t) + sizeof(len_t));
	//tlv->val = itlv->val;
	return (sizeof(tag_t) + sizeof(len_t) + tlv->len);
}

int os_tlv_get_integer(os_tlv_t *tlv, ospl_uint32 *out)
{
	if(tlv->val.pval)
	{
		ospl_uint32 *value = (ospl_uint32 *)tlv->val.pval;
		if(out)
			*out = (*value);
		return OK;
	}
	return ERROR;
}

int os_tlv_get_ospl_int16(os_tlv_t *tlv, ospl_uint16 *out)
{
	if(tlv->val.pval)
	{
		ospl_uint16 *value = (ospl_uint16 *)tlv->val.pval;
		if(out)
			*out = (*value);
		return OK;
	}
	return ERROR;
}


int os_tlv_get_byte(os_tlv_t *tlv, ospl_uint8 *out)
{
	{
		if(tlv->val.pval)
		{
			ospl_uint8 *value = (ospl_uint8 *)tlv->val.pval;
			if(out)
				*out = (*value);
			return OK;
		}
		return ERROR;
	}
}
