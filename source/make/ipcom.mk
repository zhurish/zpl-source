ifeq ($(ZPL_IPCOM_MODULE),true)

export IPCOM_DEF = -DIPCOM_DRIVER_CB -DIPCOM_DRV_ETH_HOOK -DIPUTIL -DIPCOM_OS_THREAD -DIPCOM_THREAD \
	-DIPCOM_NETSNMP -DNETSNMP -DIPSNMP -DIPCOM_USE_MIB2 -DIPNET6 -DIPCOM_USE_INET6 \
	-DIPNET_USE_BOND -DIPNET -DIPCOM_USE_INET -DIPVRRP -DIPNET_MSP_DISTRIBUTION \
	-DIPNET_STATISTICS -DIPTCP -DIPSCTP -DIPPPP -DIPIPSEC2 -DIPFIREWALL -DIP8021X \
	-DIPDHCPC -DIPDNSC -DIPTFTPC -DIPTFTPS -DIPFTPC -DIPFTPS -DIPCRYPTO \
	-DIPCRYPTO_USE_TYPE_MAPPING -DIPDHCPC6 -DIPDHCPR -DIPDHCPS6 -DIPDHCPS -DIPDIAMETER \
	-DIPEAP -DIPIKE -DIPCOM_USE_FLOAT -DIPL2TP -DIPMCP -DIPMIP4 -DIPMIP6HA  -DIPMIP6MN \
	-DIPMIP6 -DIPMIPFA -DIPMIPHA -DIPMIPMN -DIPMPLS -DIPMPLS_MULTIPLE_ROUTE_TABLES \
	-DIPRADIUS -DIPRIPNG -DIPRIP -DIPSNTP -DIPSSH -DIPSSL -DIPRIPNG -DIPDIAMETER -DIPWLAN \
	-DIPMIPPM -DIPMIP -DIPNET_USE_NETLINKSOCK -DIPROHC -DIPWPS -DIPCOM_DRV_ETH_SIMULATION 

export IPCOM_INCLUDE = -I$(ZPL_IPCOM_BASE_PATH)/ipcom/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcom/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcom/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcom/port/linux/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcom/port/linux/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcom/port/linux/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/iputil/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/iputil/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/osconfig/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/src/mib \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/snmplib \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/snmplib/transports \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/agent \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/agentx \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/helpers \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/mibII \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/snmpv3 \
				  -I$(ZPL_IPCOM_BASE_PATH)/netsnmp/netsnmp/src/util_funcs \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipnet2/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipnet2/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipnet2/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/iptcp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/iptcp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/iptcp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsctp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsctp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsctp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipppp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipppp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipppp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipipsec2/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipipsec2/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipipsec2/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipfirewall/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipfirewall/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipfirewall/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ip8021x/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ip8021x/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwlan/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipappl/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipappl/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcrypto/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcrypto/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcrypto/openssl-0_9_8/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcrypto/openssl-0_9_8/crypto \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipcrypto/openssl-0_9_8  \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpc6/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpc6/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpc6/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpr/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpr/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps6/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps6/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipeap/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipeap/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipike/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipike/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipl2tp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipl2tp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipl2tp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmcp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmcp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip4/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip4/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6ha/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6ha/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6ha/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6mn/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6mn/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6mn/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip6/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipfa/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipfa/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipfa/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipha/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipha/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipha/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipmn/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipmn/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmipmn/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmpls/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmpls/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmpls/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipradius/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipradius/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/iprip/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/iprip/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsntp/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsntp/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipssh/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipssh/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipssl2/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipssl2/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipssl2/openssl-0_9_8/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipripng/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdiameter/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwlan/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwlan/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmippm/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmippm/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmippm/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmip/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/iprohc/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/iprohc/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/iprohc/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwps/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwps/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/simulation/config \
				  -I$(ZPL_IPCOM_BASE_PATH)/simulation/include \
				  -I$(ZPL_IPCOM_BASE_PATH)/simulation/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipappl/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcpr/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps6/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipdhcps/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipl2tp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipmcp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipradius/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipsntp/src \
				  -I$(ZPL_IPCOM_BASE_PATH)/ipwlan/src

export IPCOM_LIBDIR = $(ZPL_IPCOM_BASE_PATH)/debug/lib
endif
#