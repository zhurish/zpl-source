#ifndef __OS_NETSERVICE_H__
#define __OS_NETSERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif


struct zpl_osnet_service
{
    int slot;
    int vty_port;
    int telnet_port;
    int tftpd_port;   
    int ftpd_port;  
    int sntpd_port;
    int zclient_port;  
    int standbyd_port;
    int standbyc_port;
    int ipbc_port;  
    int hal_port;  
    int pal_port; 
    int rtsp_port;   
    int web_port;  
    int web_sslport;
    int sshd_port; 
    int dhcpd_port;  
    int dhcpc_port;  
    int dhcpd6_port;  
    int dhcpc6_port;  
    int modbus_port;  
    int mqtt_port;       
};

extern int os_netservice_port_get(char *name);
extern char * os_netservice_sockpath_get(char *name);

extern int os_netservice_config_write(char *filename);
extern int os_netservice_config_load(char *filename);

#ifdef __cplusplus
}
#endif

#endif /* __OS_NETSERVICE_H__ */