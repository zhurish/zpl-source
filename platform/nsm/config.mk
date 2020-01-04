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
OBJS	+= nsm_mac.o
OBJS	+= nsm_vlan.o

OBJS	+= nsm_arp.o
OBJS	+= nsm_qos.o
OBJS	+= nsm_security.o
OBJS	+= nsm_trunk.o
OBJS	+= nsm_dos.o
OBJS	+= nsm_8021x.o
OBJS	+= nsm_mirror.o
OBJS	+= nsm_serial.o
OBJS	+= nsm_dhcp.o
OBJS	+= nsm_dhcpc.o
OBJS	+= nsm_dhcps.o
OBJS	+= nsm_tunnel.o
OBJS	+= nsm_bridge.o
OBJS	+= nsm_veth.o
OBJS	+= nsm_dns.o
OBJS	+= nsm_firewalld.o

OBJS	+= nsm_main.o
#############################################################################
# LIB
###########################################################################
LIBS = libnsm.a
