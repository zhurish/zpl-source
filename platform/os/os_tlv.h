/*
 * os_tlv.h
 *
 *  Created on: 2018��12��19��
 *      Author: DELL
 */

#ifndef _OS_TLV_H__
#define _OS_TLV_H__

#ifdef __cplusplus

extern "C" {

#endif


#include <endian.h>

typedef unsigned char val_t;
typedef unsigned int tag_t;
typedef unsigned int len_t;


//���TLV������ݽṹ
#pragma pack(1)
typedef struct os_tlv_s
{
	tag_t tag;
	len_t len;  		//��big_endianģʽ���棬ע����Ҫ�ʵ�ת��
	union
	{
		unsigned char 	*pval;
		unsigned char 	val8;
		unsigned short 	val16;
		unsigned int 	val32;
		//val_t *val;  	//���ȿɱ�
	}val;
} os_tlv_t;

#pragma pack()


extern int os_tlv_set_string(char *input, tag_t tag, len_t len, void * val);
extern int os_tlv_set_integer(char *input, tag_t tag, len_t len, void * val);
extern int os_tlv_set_zero(char *input, tag_t tag, len_t len);
extern int os_tlv_get(char *input, os_tlv_t *tlv);
extern int os_tlv_value_get(char *input, os_tlv_t *tlv);

#ifdef __cplusplus

}

#endif


#endif /* _OS_TLV_H__ */
