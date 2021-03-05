
#ifdef __cplusplus
extern "C" {
#endif

extern ospl_uint16 in_cksum(void *, ospl_uint32);
#define FLETCHER_CHECKSUM_VALIDATE 0xffff
extern ospl_uint16 fletcher_checksum(ospl_uchar *, const ospl_size_t len, const ospl_uint16 offset);
extern ospl_uint16 crc_checksum(ospl_uchar * message, ospl_uint32  nBytes);
extern ospl_uint16 crc16_ccitt(ospl_uint16 crc_start, ospl_uchar *buf, ospl_uint32 len);
 
#ifdef __cplusplus
}
#endif
