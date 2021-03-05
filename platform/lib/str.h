/*
 * $Id: str.h,v 1.4 2005/09/19 09:53:21 hasso Exp $
 */

#ifndef _ZEBRA_STR_H
#define _ZEBRA_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_SNPRINTF
extern int snprintf(ospl_char *, ospl_size_t, const char *, ...);
#endif

#ifndef HAVE_VSNPRINTF
#define vsnprintf(buf, size, format, args) vsprintf(buf, format, args)
#endif

#ifndef HAVE_STRLCPY
extern ospl_size_t strlcpy(ospl_char *, const char *, ospl_size_t);
#endif

#ifndef HAVE_STRLCAT
extern ospl_size_t strlcat(ospl_char *, const char *, ospl_size_t);
#endif

#ifndef HAVE_STRNLEN
extern ospl_size_t strnlen(const char *s, ospl_size_t maxlen);
#endif

#ifndef HAVE_STRNDUP
extern ospl_char * strndup (const char *, ospl_size_t);
#endif

extern const char *strupr(ospl_char* src);
extern const char *strlwr(ospl_char* src);
extern const char *string_have_space(ospl_char* src);
extern int all_space (const char *str);
extern const char *str_trim(ospl_char* src);

extern const char *itoa(int value, int base);
extern const char *ftoa(ospl_float value, ospl_char *fmt);
extern const char *dtoa(ospl_double value, ospl_char *fmt);

extern ospl_uint8 atoascii(int a);
extern ospl_bool is_hex (ospl_char c);
extern ospl_uint32 str_to_hex(ospl_char * room);
extern ospl_char * hex_to_str(ospl_uint32 hex);


extern int strchr_count(ospl_char *src, const char em);

extern int strchr_step(ospl_char *src, const char em, int step);
extern int strchr_next(ospl_char *src, const char em);

extern ospl_char *strstr_last(const char *dest,const char *src);

extern int str_isempty(ospl_char *dest, ospl_uint32 len);

extern ospl_char *os_strstr_last(const char *dest,const char *src);


extern ospl_uint32 getULong(ospl_uchar *);
extern ospl_int32 getLong(ospl_uchar *);
extern ospl_uint16 getUShort(ospl_uchar *);
extern ospl_int16 getShort(ospl_uchar *);
extern void putULong(ospl_uchar *, ospl_uint32);
extern void putLong(ospl_uchar *, ospl_int32);
extern void putUShort(ospl_uchar *, ospl_int16 );
extern void putShort(ospl_uchar *, ospl_int16);

extern void convert_num(ospl_uchar *buf, ospl_char *str, int base, int size);


extern long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);
 
#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_STR_H */

