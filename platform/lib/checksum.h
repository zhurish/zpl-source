
#ifdef __cplusplus
extern "C" {
#endif

extern zpl_uint16 in_cksum(void *, zpl_uint32);
#define FLETCHER_CHECKSUM_VALIDATE 0xffff
extern zpl_uint16 fletcher_checksum(zpl_uchar *, const zpl_size_t len, const zpl_uint16 offset);
extern zpl_uint16 crc_checksum(zpl_uchar * message, zpl_uint32  nBytes);
extern zpl_uint16 crc16_ccitt(zpl_uint16 crc_start, zpl_uchar *buf, zpl_uint32 len);
 
#ifdef __cplusplus
}
#endif
