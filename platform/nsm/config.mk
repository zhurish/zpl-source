#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/nsm
#nsm
ifeq ($(strip $(PL_NSM_MODULE)),true)
OBJS	+= nsm_interface.o
OBJS	+= nsm_connected.o
OBJS	+= nsm_client.o
OBJS	+= nsm_hook.o
OBJS	+= nsm_router-id.o


#OBJS	+= nsm_zebra_routemap.o
#OBJS	+= nsm_snmp.o
#OBJS	+= nsm_vty.o

OBJS	+= nsm_distribute.o
OBJS	+= nsm_redistribute.o
OBJS	+= nsm_filter.o
OBJS	+= nsm_if_rmap.o
OBJS	+= nsm_keychain.o
OBJS	+= nsm_plist.o
OBJS	+= nsm_routemap.o

OBJS	+= nsm_vrf.o
OBJS	+= nsm_rib.o
OBJS	+= nsm_rnh.o
OBJS	+= nsm_fpm.o
OBJS	+= nsm_zserv.o
OBJS	+= nsm_debug.o



ifeq ($(strip $(PL_NSM_8021X)),true)
OBJS	+= nsm_8021x.o			
endif
ifeq ($(strip $(PL_NSM_ARP)),true)
OBJS	+= nsm_arp.o			
endif
ifeq ($(strip $(PL_NSM_BRIDGE)),true)
OBJS	+= nsm_bridge.o			
endif
ifeq ($(strip $(PL_NSM_DHCP)),true)
OBJS	+= nsm_dhcp.o
OBJS	+= nsm_dhcpc.o
OBJS	+= nsm_dhcps.o			
endif
ifeq ($(strip $(PL_NSM_DNS)),true)
OBJS	+= nsm_dns.o			
endif
ifeq ($(strip $(PL_NSM_DOS)),true)
OBJS	+= nsm_dos.o			
endif
ifeq ($(strip $(PL_NSM_FIREWALLD)),true)
OBJS	+= nsm_firewalld.o			
endif
ifeq ($(strip $(PL_NSM_MAC)),true)
OBJS	+= nsm_mac.o		
endif
ifeq ($(strip $(PL_NSM_VLAN)),true)
OBJS	+= nsm_vlan.o		
endif
ifeq ($(strip $(PL_NSM_QOS)),true)
OBJS	+= nsm_qos.o		
endif
ifeq ($(strip $(PL_NSM_TRUNK)),true)
OBJS	+= nsm_trunk.o		
endif
ifeq ($(strip $(PL_NSM_MIRROR)),true)
OBJS	+= nsm_mirror.o		
endif
ifeq ($(strip $(PL_NSM_TUNNEL)),true)
OBJS	+= nsm_tunnel.o		
endif
ifeq ($(strip $(PL_NSM_SERIAL)),true)
OBJS	+= nsm_serial.o	
endif
ifeq ($(strip $(PL_NSM_PPP)),true)
OBJS	+= nsm_ppp.o	
endif
ifeq ($(strip $(PL_NSM_SECURITY)),true)
OBJS	+= nsm_security.o	
endif

ifeq ($(strip $(PL_NSM_VETH)),true)
OBJS	+= nsm_veth.o	
endif


OBJS	+= nsm_main.o
#############################################################################
# LIB
###########################################################################
LIBS = libnsm.a
endif