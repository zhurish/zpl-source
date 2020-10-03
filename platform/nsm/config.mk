#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/nsm
#nsm
#OBJS	+= debug.o
OBJS	+= interface.o
OBJS	+= redistribute.o
OBJS	+= router-id.o
OBJS	+= connected.o
#OBJS	+= zebra_fpm.o
#OBJS	+= zebra_rib.o

#OBJS	+= zebra_routemap.o
#OBJS	+= zebra_snmp.o
#OBJS	+= zebra_vty.o

OBJS	+= distribute.o
OBJS	+= filter.o
#OBJS	+= if_rmap.o
OBJS	+= keychain.o
OBJS	+= plist.o
OBJS	+= routemap.o

OBJS	+= vrf.o
#OBJS	+= nsm_log.o
OBJS	+= zebra_rib.o
OBJS	+= zebra_rnh.o
OBJS	+= zserv.o
#OBJS	+= zclient.o
OBJS	+= debug.o

OBJS	+= nsm_client.o
OBJS	+= nsm_hook.o


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
