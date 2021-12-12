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

typedef zpl_uint8 val_t;
typedef zpl_uint32 tag_t;
typedef zpl_uint32 len_t;


//���TLV������ݽṹ
#pragma pack(1)
typedef struct os_tlv_s
{
	tag_t tag;
	len_t len;  		//��big_endianģʽ���棬ע����Ҫ�ʵ�ת��
	union
	{
		zpl_uint8 	*pval;
		zpl_uint8 	val8;
		zpl_uint16 	val16;
		zpl_uint32 	val32;
		//val_t *val;  	//���ȿɱ�
	}val;
} os_tlv_t;

#pragma pack()


extern int os_tlv_set_string(char *input, tag_t tag, len_t len, void * val);
extern int os_tlv_set_integer(char *input, tag_t tag, len_t len, void * val);
extern int os_tlv_set_zero(char *input, tag_t tag, len_t len);
extern int os_tlv_set_octet(char *input, tag_t tag, len_t len, void * val);

extern int os_tlv_get(char *input, os_tlv_t *tlv);
extern int os_tlv_value_get(char *input, os_tlv_t *tlv, len_t len);

extern int os_tlv_get_integer(os_tlv_t *tlv, zpl_uint32 *out);
extern int os_tlv_get_zpl_int16(os_tlv_t *tlv, zpl_uint16 *out);
extern int os_tlv_get_byte(os_tlv_t *tlv, zpl_uint8 *out);


#ifdef __cplusplus
}
#endif


#endif /* _OS_TLV_H__ */
