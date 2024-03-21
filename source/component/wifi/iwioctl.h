/*
 * wifi.h
 *
 *  Created on: Jul 15, 2018
 *      Author: zhurish
 */

#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "iw_config.h"
#include "iw_ap.h"
#include "iw_client.h"

#ifndef CONFIG_IW_TOOLS
#define WIFI_FILE_NAME	".iw-local.info"
#define WIFI_FILE_NAME_MAX	64
#endif



typedef struct iw_tmp_s
{
	ifindex_t	ifindex;
#ifndef CONFIG_IW_TOOLS
	char	filename[WIFI_FILE_NAME_MAX];
#endif
	struct vty *vty;

}iw_tmp_t;


extern const char *wifi_file_path(void);
extern const char * kname2ifname(const char *kname);


extern int iw_printf(const char *format, ...);
extern int iw_fprintf(void *fp, const char *format, ...);


extern int iw_dev_mode_set(struct interface *ifp, char * value);
extern int iw_dev_txpower_set(struct interface *ifp, char * value);
extern int iw_dev_rts_threshold_set(struct interface *ifp, char * value);
extern int iw_dev_frag_set(struct interface *ifp, char * value);
extern int iw_dev_rate_set(struct interface *ifp, char * value);
extern int iw_dev_bit_set(struct interface *ifp, char * value);
extern int iw_dev_channel_set(struct interface *ifp, char * value);

extern int iw_ap_dev_information_show(struct interface *ifp, struct vty *vty);
extern iw_mode_t iw_ap_dev_mode_get(struct interface *ifp);

/*
 * show iw phy capabilities
 */
extern int iw_phy_capabilities_show(struct interface *ifp, struct vty *vty);
/*
 * show support channel/freq
 */
extern int iw_dev_channel_support_show(struct interface *ifp, struct vty *vty);

extern int iw_get_freq_info(char * ifname, iw_dev_t *iwdev);
extern int iw_get_bitrate_info(char * ifname, iw_dev_t *iwdev);


extern int iw_client_dev_station_dump_show(struct interface *ifp, struct vty *vty);
/*
 * scanning AP
 */
extern int iw_client_dev_scan_ap_show(struct interface *ifp, struct vty *vty, zpl_bool detail);
/*
 * show current connect information
 */
extern int iw_client_dev_connect_show(struct interface *ifp, struct vty *vty);
extern int iw_client_dev_connect_get(struct interface *ifp, zpl_uint8 essid[]);

extern int iw_client_dev_connect(struct interface *ifp, iw_client_ap_t *ap, char *ssid, char *password);
extern int iw_client_dev_start_dhcpc(struct interface *ifp);
extern int iw_client_dev_disconnect(struct interface *ifp);
extern int iw_client_dev_is_connect(char *ifname, zpl_uint8 *bssid);



extern int iw_client_scan_process(iw_client_t *iw_client);
extern int iw_client_scan_process_exit(iw_client_t *iw_client);


extern int iw_ap_connect_scanning(iw_ap_t *iw_ap);

extern int iw_dev_mode(struct interface *ifp);


 
#ifdef __cplusplus
}
#endif
 
#endif /* __WIFI_H__ */
