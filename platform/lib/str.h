/*
 * $Id: str.h,v 1.4 2005/09/19 09:53:21 hasso Exp $
 */

#ifndef _ZEBRA_STR_H
#define _ZEBRA_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_SNPRINTF
extern int snprintf(zpl_char *, zpl_size_t, const char *, ...);
#endif

#ifndef HAVE_VSNPRINTF
#define vsnprintf(buf, size, format, args) vsprintf(buf, format, args)
#endif

#ifndef HAVE_STRLCPY
extern zpl_size_t strlcpy(zpl_char *, const char *, zpl_size_t);
#endif

#ifndef HAVE_STRLCAT
extern zpl_size_t strlcat(zpl_char *, const char *, zpl_size_t);
#endif

#ifndef HAVE_STRNLEN
extern zpl_size_t strnlen(const char *s, zpl_size_t maxlen);
#endif

#ifndef HAVE_STRNDUP
extern zpl_char * strndup (const char *, zpl_size_t);
#endif

extern const char *strupr(zpl_char* src);
extern const char *strlwr(zpl_char* src);
extern const char *string_have_space(zpl_char* src);
extern int all_space (const char *str);
extern int all_isdigit (const char *str);
extern const char *str_trim(zpl_char* src);

extern const char *itoa(int value, int base);
extern const char *ftoa(zpl_float value, zpl_char *fmt);
extern const char *dtoa(zpl_double value, zpl_char *fmt);

extern zpl_uint8 atoascii(int a);
extern zpl_bool is_hex (zpl_char c);
extern zpl_uint32 str_to_hex(zpl_char * room);
extern zpl_char * hex_to_str(zpl_uint32 hex);


extern int strchr_count(zpl_char *src, const char em);
/*获取字符的数量，返回最后一个的位置*/
extern int strchr_step(zpl_char *src, const char em, int step);
/*获取字符的偏移位置*/
extern int strchr_offset(zpl_char *src, const char em);
/*获取连续两个字符的间隔*/
extern int strchr_step_num(zpl_char *src, const char em);

extern zpl_char *strstr_last(const char *dest,const char *src);

extern int str_isempty(zpl_char *dest, zpl_uint32 len);

extern zpl_char *os_strstr_last(const char *dest,const char *src);


extern zpl_uint32 getULong(zpl_uchar *);
extern zpl_int32 getLong(zpl_uchar *);
extern zpl_uint16 getUShort(zpl_uchar *);
extern zpl_int16 getShort(zpl_uchar *);
extern void putULong(zpl_uchar *, zpl_uint32);
extern void putLong(zpl_uchar *, zpl_int32);
extern void putUShort(zpl_uchar *, zpl_int16 );
extern void putShort(zpl_uchar *, zpl_int16);

extern void convert_num(zpl_uchar *buf, zpl_char *str, int base, int size);


extern long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_STR_H */

