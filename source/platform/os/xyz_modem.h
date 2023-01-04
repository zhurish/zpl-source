/*
 * xyz_modem.h
 *
 *  Created on: Apr 18, 2019
 *      Author: zhurish
 */

#ifndef __XYZ_MODEM_H__
#define __XYZ_MODEM_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "zpl_type.h"

#define xyzModem_xmodem 1
#define xyzModem_ymodem 2
/* Don't define this until the protocol support is in place */
/*#define xyzModem_zmodem 3 */

#define xyzModem_access   -1
#define xyzModem_noZmodem -2
#define xyzModem_timeout  -3
#define xyzModem_eof      -4
#define xyzModem_cancel   -5
#define xyzModem_frame    -6
#define xyzModem_cksum    -7
#define xyzModem_sequence -8

#define xyzModem_close 1
#define xyzModem_abort 2


#define CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT
#define CYGACC_CALL_IF_SET_CONSOLE_COMM(x)

/*
#define diag_vprintf vprintf
#define diag_printf printf
#define diag_vsprintf vsprintf
*/

#define CYGACC_CALL_IF_DELAY_US(x) os_usleep(x)


/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#undef EOF
#define EOF 0x1A		/* ^Z for DOS officionados */


#define XYZ_MIN_SIZE 128
#define XYZ_MAX_SIZE 1024

#pragma pack(1)
typedef struct xyz_modem_hdr_s
{
    zpl_uint8 code;
    zpl_uint8 sequm;
    zpl_uint8 sequm_inv;
    zpl_uint8 data[XYZ_MIN_SIZE];
    zpl_uint8 crc_h;
    zpl_uint8 crc_l;
}xyz_modem_hdr_t, xyz_modem_data_last_t;

typedef struct xyz_modem_data_s
{
    zpl_uint8 code;
    zpl_uint8 sequm;
    zpl_uint8 sequm_inv;
    zpl_uint8 data[XYZ_MAX_SIZE];
    zpl_uint8 crc_h;
    zpl_uint8 crc_l;
}xyz_modem_data_t;
#pragma pack(0)

/* Data & state local to the protocol */
typedef struct
{
  zpl_uint8 sequm;

  zpl_uint8 s_eof, s_ack, s_nak;

  int fd;
  zpl_uchar pkt[XYZ_MAX_SIZE], *bufp;
  zpl_uchar blk, cblk, crc1, crc2;
  zpl_uchar next_blk;	/* Expected block */
  zpl_uint32 len, mode, total_retries;
  zpl_uint32 total_SOH, total_STX, total_CAN;
  zpl_bool crc_mode, at_eof, tx_ack;
  zpl_uint32 file_length, read_length;

  /*
   * recv data callback
   */
  void		*pravi;
  int		(*data_cb)(void *, zpl_uchar *, zpl_uint32);

  /*
   * debug info show callback
   */
  void		*vty;
  int		(*show_debug)(void *, char *,...);

  /*
   * xy modem hw control function callback
   */
  int		(*write_cb)(int, zpl_uchar *, zpl_uint32);
  int		(*write_wait_cb)(int, zpl_uchar *, zpl_uint32, zpl_uint32);
  int		(*read_cb)(int, zpl_uchar *, zpl_uint32);

  int		(*putc_cb)(int, zpl_uint32);
  int		(*getc_cb)(int, zpl_uchar *, zpl_uint32);


} xyz_modem_t;

#define xyzModem_CHAR_TIMEOUT            3000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             10
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */
#define xyzModem_PACK_TIMEOUT            3000	/* 2 seconds */

extern int xyz_modem_build_hdr(xyz_modem_t*xyz, xyz_modem_hdr_t *hdr, zpl_char *filename, zpl_uint32 filesize);
extern int xyz_modem_build_data(xyz_modem_t*xyz, xyz_modem_data_t *hdr, zpl_uchar *data, zpl_uint32 len);
extern int xyz_modem_build_data_last(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr, zpl_uchar *data, zpl_uint32 len);
extern int xyz_modem_build_finsh_empty(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr);;
extern int xyz_modem_build_finsh_eot(xyz_modem_t*xyz, zpl_uint32);

extern zpl_char * xyz_modem_error(zpl_uint32 err);
extern int xyz_modem_load(xyz_modem_t *xyz, zpl_uint32 mode, zpl_char *devi);


#ifdef __cplusplus
}
#endif

#endif /* __XYZ_MODEM_H__ */