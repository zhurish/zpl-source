#############################################################################
# DEFINE
###########################################################################
MODULEDIR = cli/nsm
#OS
OBJS	+= cmd_interface.o
OBJS	+= cmd_routerid.o
OBJS	+= cmd_vrf.o
OBJS	+= cmd_port.o
OBJS	+= cmd_route.o
OBJS	+= cmd_wireless.o

ifeq ($(strip $(PL_NSM_8021X)),true)
OBJS	+= cmd_dot1x.o			
endif
ifeq ($(strip $(PL_NSM_ARP)),true)
OBJS	+= cmd_arp.o			
endif
ifeq ($(strip $(PL_NSM_BRIDGE)),true)
OBJS	+= cmd_bridge.o		
endif
ifeq ($(strip $(PL_NSM_DHCP)),true)		
endif
ifeq ($(strip $(PL_NSM_DNS)),true)
OBJS	+= cmd_dns.o			
endif
ifeq ($(strip $(PL_NSM_DOS)),true)
OBJS	+= cmd_dos.o			
endif
ifeq ($(strip $(PL_NSM_FIREWALLD)),true)
OBJS	+= cmd_firewalld.o			
endif
ifeq ($(strip $(PL_NSM_MAC)),true)
OBJS	+= cmd_mac.o	
endif
ifeq ($(strip $(PL_NSM_VLAN)),true)
OBJS	+= cmd_vlan.o		
endif
ifeq ($(strip $(PL_NSM_QOS)),true)
OBJS	+= cmd_qos.o	
endif
ifeq ($(strip $(PL_NSM_TRUNK)),true)
OBJS	+= cmd_trunk.o		
endif
ifeq ($(strip $(PL_NSM_MIRROR)),true)
OBJS	+= cmd_mirror.o	
endif
ifeq ($(strip $(PL_NSM_TUNNEL)),true)
OBJS	+= cmd_tunnel.o	
endif
ifeq ($(strip $(PL_NSM_SERIAL)),true)
OBJS	+= cmd_serial.o
endif
ifeq ($(strip $(PL_NSM_PPP)),true)
OBJS	+= cmd_ppp.o	
endif
ifeq ($(strip $(PL_NSM_SECURITY)),true)
endif
ifeq ($(strip $(PL_NSM_VETH)),true)
endif

#############################################################################
# LIB
###########################################################################
LIBS = libclinsm.a
