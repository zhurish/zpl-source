/*
 * os_file.h
 *
 *  Created on: Jul 17, 2018
 *      Author: zhurish
 */

#ifndef __OS_FILE_H__
#define __OS_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif


extern int os_mkdir(const zpl_char *dirpath, zpl_uint32 mode, zpl_uint32 pathflag);
extern int os_rmdir(const zpl_char *dirpath, zpl_uint32 pathflag);
extern int os_getpwddir(const zpl_char *path, zpl_uint32 pathsize);


extern int os_file_access(zpl_char *filename);

extern int os_write_file(const zpl_char *name, const zpl_char *string, zpl_uint32 len);
extern int os_read_file(const zpl_char *name, const zpl_char *string, zpl_uint32 len);


/*
 * FILE SIZE
 */
#define MPLS_1_M(n)	(n << 20)
#define MPLS_1_K(n)	(n << 10)

#define MPLS_X_B(n)	(n)
#define MPLS_X_K(n)	(n >> 10)
#define MPLS_X_M(n)	(n >> 20)
#define MPLS_X_G(n)	(n >> 30)

extern int os_file_size (const zpl_char *filename);
extern const zpl_char * os_file_size_string(zpl_ullong len);



#ifdef __cplusplus
}
#endif

#endif /* __OS_FILE_H__ */
