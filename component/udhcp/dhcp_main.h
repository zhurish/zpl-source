#ifndef __DHCP_MAIN_H__
#define __DHCP_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif


int udhcp_read_thread(struct eloop *eloop);


int udhcpd_main_a(void *p);

 
#ifdef __cplusplus
}
#endif
 
#endif /*__DHCP_MAIN_H__*/
