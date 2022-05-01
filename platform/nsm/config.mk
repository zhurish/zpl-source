#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/nsm
#nsm
ifeq ($(strip $(ZPL_NSM_MODULE)),true)

OBJS	+= nsm_debug.o
OBJS	+= nsm_redistribute.o
OBJS	+= nsm_rib.o

OBJS	+= nsm_zebra_routemap.o
OBJS	+= nsm_rnh.o
OBJS	+= nsm_zserv.o

OBJS	+= nsm_interface.o
OBJS	+= nsm_halpal.o
#OBJS	+= nsm_client.o
#OBJS	+= nsm_hook.o
#OBJS	+= nsm_rib.o
OBJS	+= nsm_global.o
OBJS	+= nsm_port.o

#OBJS	+= moduletable.o
#ICMP Router Discovery Messages
ifeq ($(strip $(ZPL_NSM_IRDP)),true)
OBJS	+= nsm_irdp_interface.o		
OBJS	+= nsm_irdp_packet.o	
OBJS	+= nsm_irdp_main.o		
endif

#Router advertisement
ifeq ($(strip $(ZPL_NSM_RTADV)),true)
OBJS	+= nsm_rtadv.o		
endif

ifeq ($(strip $(ZPL_NSM_8021X)),true)
OBJS	+= nsm_8021x.o			
endif
ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= nsm_arp.o			
endif
ifeq ($(strip $(ZPL_NSM_BRIDGE)),true)
OBJS	+= nsm_bridge.o			
endif
ifeq ($(strip $(ZPL_NSM_DHCP)),true)
OBJS	+= nsm_dhcp.o
OBJS	+= nsm_dhcpc.o
OBJS	+= nsm_dhcps.o			
endif
ifeq ($(strip $(ZPL_NSM_DNS)),true)
OBJS	+= nsm_dns.o			
endif
ifeq ($(strip $(ZPL_NSM_DOS)),true)
OBJS	+= nsm_dos.o			
endif
ifeq ($(strip $(ZPL_NSM_FIREWALLD)),true)
OBJS	+= nsm_firewalld.o			
endif
ifeq ($(strip $(ZPL_NSM_MAC)),true)
OBJS	+= nsm_mac.o		
endif
ifeq ($(strip $(ZPL_NSM_VLAN)),true)
OBJS	+= nsm_vlan.o		
endif
ifeq ($(strip $(ZPL_NSM_QOS)),true)
OBJS	+= nsm_qos.o
OBJS	+= nsm_qos_acl.o		
endif
ifeq ($(strip $(ZPL_NSM_TRUNK)),true)
OBJS	+= nsm_trunk.o		
endif
ifeq ($(strip $(ZPL_NSM_MIRROR)),true)
OBJS	+= nsm_mirror.o		
endif
ifeq ($(strip $(ZPL_NSM_TUNNEL)),true)
OBJS	+= nsm_tunnel.o		
endif
ifeq ($(strip $(ZPL_NSM_SERIAL)),true)
OBJS	+= nsm_serial.o	
endif
ifeq ($(strip $(ZPL_NSM_PPP)),true)
OBJS	+= nsm_ppp.o	
endif
ifeq ($(strip $(ZPL_NSM_SECURITY)),true)
OBJS	+= nsm_security.o	
endif

ifeq ($(strip $(ZPL_NSM_VLANETH)),true)
OBJS	+= nsm_vlaneth.o	
endif

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS	+= cmd_debug.o
OBJS	+= cmd_interface.o
OBJS	+= cmd_routerid.o
OBJS	+= cmd_vrf.o
OBJS	+= cmd_port.o
ifeq ($(strip $(ZPL_NSM_L3MODULE)),true)
OBJS	+= cmd_route.o
endif

ifeq ($(strip $(ZPL_NSM_WIRELESS)),true)
OBJS	+= cmd_wireless.o
endif

OBJS	+= cmd_global.o

ifeq ($(strip $(ZPL_NSM_8021X)),true)
OBJS	+= cmd_dot1x.o			
endif
ifeq ($(strip $(ZPL_NSM_ARP)),true)
OBJS	+= cmd_arp.o			
endif
ifeq ($(strip $(ZPL_NSM_BRIDGE)),true)
OBJS	+= cmd_bridge.o		
endif
ifeq ($(strip $(ZPL_NSM_DHCP)),true)
OBJS += cmd_dhcpc.o
OBJS += cmd_dhcps.o		
endif
ifeq ($(strip $(ZPL_NSM_DNS)),true)
OBJS	+= cmd_dns.o			
endif
ifeq ($(strip $(ZPL_NSM_DOS)),true)
OBJS	+= cmd_dos.o			
endif
ifeq ($(strip $(ZPL_NSM_FIREWALLD)),true)
OBJS	+= cmd_firewalld.o			
endif
ifeq ($(strip $(ZPL_NSM_MAC)),true)
OBJS	+= cmd_mac.o	
endif
ifeq ($(strip $(ZPL_NSM_VLAN)),true)
OBJS	+= cmd_vlan.o		
endif
ifeq ($(strip $(ZPL_NSM_QOS)),true)
OBJS	+= cmd_qos.o	
OBJS	+= cmd_qos_acl.o
endif
ifeq ($(strip $(ZPL_NSM_TRUNK)),true)
OBJS	+= cmd_trunk.o		
endif
ifeq ($(strip $(ZPL_NSM_MIRROR)),true)
OBJS	+= cmd_mirror.o	
endif
ifeq ($(strip $(ZPL_NSM_TUNNEL)),true)
OBJS	+= cmd_tunnel.o	
endif
ifeq ($(strip $(ZPL_NSM_SERIAL)),true)
OBJS	+= cmd_serial.o
endif
ifeq ($(strip $(ZPL_NSM_PPP)),true)
OBJS	+= cmd_ppp.o	
endif
ifeq ($(strip $(ZPL_NSM_SECURITY)),true)
OBJS	+= cmd_security.o
endif
ifeq ($(strip $(ZPL_NSM_VLANETH)),true)
endif
endif

OBJS	+= nsm_main.o
#############################################################################
# LIB
###########################################################################
LIBS = libnsm.a
endif