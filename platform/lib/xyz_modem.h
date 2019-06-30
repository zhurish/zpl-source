/*
 * xyz_modem.h
 *
 *  Created on: Apr 18, 2019
 *      Author: zhurish
 */

#ifndef __XYZ_MODEM_H__
#define __XYZ_MODEM_H__

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
#define EOF 0x1A		/* ^Z for DOS officionados */


#define XYZ_MIN_SIZE 128
#define XYZ_MAX_SIZE 1024

#pragma pack(1)
typedef struct xyz_modem_hdr_s
{
    u_int8 code;
    u_int8 sequm;
    u_int8 sequm_inv;
    u_int8 data[XYZ_MIN_SIZE];
    u_int8 crc_h;
    u_int8 crc_l;
}xyz_modem_hdr_t, xyz_modem_data_last_t;

typedef struct xyz_modem_data_s
{
    u_int8 code;
    u_int8 sequm;
    u_int8 sequm_inv;
    u_int8 data[XYZ_MAX_SIZE];
    u_int8 crc_h;
    u_int8 crc_l;
}xyz_modem_data_t;
#pragma pack(0)

/* Data & state local to the protocol */
typedef struct
{
  u_int8 sequm;

  u_int8 s_eof, s_ack, s_nak;

  int fd;
  unsigned char pkt[XYZ_MAX_SIZE], *bufp;
  unsigned char blk, cblk, crc1, crc2;
  unsigned char next_blk;	/* Expected block */
  int len, mode, total_retries;
  int total_SOH, total_STX, total_CAN;
  BOOL crc_mode, at_eof, tx_ack;
  unsigned long file_length, read_length;

  /*
   * recv data callback
   */
  void		*pravi;
  int		(*data_cb)(void *, unsigned char *, int);

  /*
   * debug info show callback
   */
  void		*vty;
  int		(*show_debug)(void *, char *,...);

  /*
   * xy modem hw control function callback
   */
  int		(*write_cb)(int, unsigned char *, int);
  int		(*write_wait_cb)(int, unsigned char *, int, int);
  int		(*read_cb)(int, unsigned char *, int);

  int		(*putc_cb)(int, int);
  int		(*getc_cb)(int, unsigned char *, int);


} xyz_modem_t;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */
#define xyzModem_PACK_TIMEOUT            5000	/* 2 seconds */

extern int xyz_modem_build_hdr(xyz_modem_t*xyz, xyz_modem_hdr_t *hdr, char *filename, int filesize);
extern int xyz_modem_build_data(xyz_modem_t*xyz, xyz_modem_data_t *hdr, char *data, int len);
extern int xyz_modem_build_data_last(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr, char *data, int len);
extern int xyz_modem_build_finsh_empty(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr);;
extern int xyz_modem_build_finsh_eot(xyz_modem_t*xyz, int);

extern char * xyz_modem_error(int err);
extern int xyz_modem_load(xyz_modem_t *xyz, int mode, char *devi);



#endif /* __XYZ_MODEM_H__ */
