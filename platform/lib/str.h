/*
 * $Id: str.h,v 1.4 2005/09/19 09:53:21 hasso Exp $
 */

#ifndef _ZEBRA_STR_H
#define _ZEBRA_STR_H

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, ...);
#endif

#ifndef HAVE_VSNPRINTF
#define vsnprintf(buf, size, format, args) vsprintf(buf, format, args)
#endif

#ifndef HAVE_STRLCPY
extern size_t strlcpy(char *, const char *, size_t);
#endif

#ifndef HAVE_STRLCAT
extern size_t strlcat(char *, const char *, size_t);
#endif

#ifndef HAVE_STRNLEN
extern size_t strnlen(const char *s, size_t maxlen);
#endif

#ifndef HAVE_STRNDUP
extern char * strndup (const char *, size_t);
#endif

extern const char *strupr(char* src);
extern const char *strlwr(char* src);
extern const char *string_have_space(char* src);
extern int all_space (const char *str);
extern const char *itoa(int value, int base);
extern const char *itof(float value);

extern u_int8 atoascii(int a);
extern BOOL is_hex (char c);
extern u_int32 string_to_hex(char * room);
extern char * hex_to_string(u_int32 hex);


extern int strchr_count(char *src, const char em);

extern int strchr_step(char *src, const char em, int step);
extern int strchr_next(char *src, const char em);

extern char *strstr_last(const char *dest,const char *src);

extern int str_isempty(char *dest, int len);

extern char *os_strstr_last(const char *dest,const char *src);


extern u_int32_t getULong(unsigned char *);
extern int32_t getLong(unsigned char *);
extern u_int16_t getUShort(unsigned char *);
extern int16_t getShort(unsigned char *);
extern void putULong(unsigned char *, u_int32_t);
extern void putLong(unsigned char *, int32_t);
extern void putUShort(unsigned char *, unsigned int);
extern void putShort(unsigned char *, int);

extern void convert_num(unsigned char *buf, char *str, int base, int size);


extern long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);

#endif /* _ZEBRA_STR_H */

