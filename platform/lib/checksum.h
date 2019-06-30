extern int in_cksum(void *, int);
#define FLETCHER_CHECKSUM_VALIDATE 0xffff
extern u_int16_t fletcher_checksum(u_char *, const size_t len, const uint16_t offset);
extern u_int16_t crc_checksum(unsigned char * message, unsigned int nBytes);
extern uint16_t crc16_ccitt(uint16_t crc_start, unsigned char *buf, int len);
