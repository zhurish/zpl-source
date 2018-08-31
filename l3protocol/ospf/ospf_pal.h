/*
 * ospf_pal.h
 *
 *  Created on: May 18, 2018
 *      Author: zhurish
 */

#ifndef __OSPF_PAL_H__
#define __OSPF_PAL_H__

typedef enum
{
    SYS_IF_MODE=1,  /*���ýӿ�ģʽ��block����forward*/
    SYS_IF_DESC,    /*�ӿ�����*/
    SYS_IF_NAME,
    SYS_IF_ALIAS,
    SYS_IF_TYPE,    /*��׼��mib2�ӿ�����ֵ��:M2_ifType_fastEther��*/
    SYS_IF_PHYTYPE, /*����ĳһ��ӿڹ���ͬһ��endtype�������Ҫ�����������־���Ľӿ����ͣ��ɽӿڳ�ʼ��ʱд��*/
    SYS_IF_ENDTYPE, /*end�ӿ�����ֵ��ȡֵ��:USP_END_FE*/
    SYS_IF_ENDNAME, /*end�ӿ����֣�ȡֵ��:fe,ge,xge,svi��*/
    SYS_IF_ENDFLAG, /*end flag*/
    SYS_IF_HIDDEN,
    SYS_IF_IGNOREHW,
    SYS_IF_IPV4FLAG,    /*ipv4 interface flag*/
    SYS_IF_IPV6FLAG,    /*ipv6 interface flag*/
    SYS_IF_SPEED,   /*�ӿ��ٶȣ������λΪbits/persecond*//*���鲻Ҫ����set����,������ھۺϽӿ��޷��������Ҫд���������ɶ�ȡ����ֵ�����ۼӴӶ����stp�����⣬����ۼ��������д����������*/

    /*
    ��������������Ϊһ�飬������SYS_IF_DUPLEXSPEED,SYS_IF_CFGDUPLEXSPEED����
    */
    SYS_IF_HIGHSPEED,   /*��ȡ�ӿڵ���Ч�ٶ�(��trunk��ֻȡ�ֹ�up��lacp bind�Ķ˿ڴ���ֵ)�������λΪ(1,000,000)Mbits/persecond(Mb/s),��������ʱҪ���ø�������*//*���鲻Ҫ����set����,������ھۺϽӿ��޷��������Ҫд���������ɶ�ȡ����ֵ�����ۼӴӶ����stp�����⣬����ۼ��������д����������*/
    SYS_IF_DUPLEX,  /*�ӿ�˫��ģʽ*/
    SYS_IF_AUTONEGO,    /*�ӿ���Э��ģʽ,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_AUTONEGOCOMBO,   /*�ӿ���Э��ģʽ,�����combo������Ϊ�ǵ��,��combo����Ч.*/
    SYS_IF_AUTONEGOSUPPORT,/*�Ƿ�֧����Э��,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_AUTONEGOSUPPORTCOMBO,    /*�Ƿ�֧����Э��,�����combo������Ϊ�ǵ��,��combo����Ч.*/

    SYS_IF_MAUTYPE,     /*��̫����ӿ�mautype����������octetstring, ֻ����
                ÿλ�ĺ���μ�rfc3636,rfc4836,ianamautype,ifMauTypeListBits,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/

    SYS_IF_MAUTYPECOMBO,    /*��̫����ӿ�mautype����������octetstring, ֻ����
                ÿλ�ĺ���μ�rfc3636,rfc4836,ianamautype,ifMauTypeListBits,�����combo������Ϊ�ǵ��,��combo����Ч.*/

    SYS_IF_MAUDFLTTYPE,     /*��̫����ӿ�mautype����������int*, ��/д��
                ÿλ�ĺ���μ�rfc3636,rfc4836,ianamautype,ifMauTypeListBits,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/

    SYS_IF_MAUDFLTTYPECOMBO,    /*��̫����ӿ�mautype����������int*, ��/д��
                ÿλ�ĺ���μ�rfc3636,rfc4836,ianamautype,ifMauTypeListBits,�����combo������Ϊ�ǵ��,��combo����Ч.*/

    SYS_IF_CAPABILITY,  /*�ӿڵ���������������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_CAPABILITYCOMBO, /*�ӿڵ���������������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǵ��,��combo����Ч.*/
    SYS_IF_LOCALADVERTISE,  /*���ñ���˫��������ͨ�����ԡ���������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_LOCALADVERTISECOMBO,     /*���ñ���˫��������ͨ�����ԡ���������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǵ��,��combo����Ч.*/
    SYS_IF_PEERADVERTISE,   /*��ȡ�Զ˽ӿ�˫��������ͨ�����ԡ���������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_PEERADVERTISECOMBO,  /*��ȡ�Զ˽ӿ�˫��������ͨ�����ԡ���������bitstring,ֻ����ÿλ�Ķ����IANAifMauAutoNegCapBits,�����combo������Ϊ�ǵ��,��combo����Ч.*/

    SYS_IF_OPERSTATUS,  /*�ӿڲ���״̬*/
    SYS_IF_ADMINSTATUS, /*�ӿڹ���״̬*/
    SYS_IF_PHYADDR, /*�ӿ������ַ�����޸ġ��ɱ�HA��ѵ�Э��Э�̺�ͨ���޸�*/
    SYS_IF_ORIGINALPHYADDR, /*ԭʼ�����ַ�������޸�*/
    SYS_IF_PHYADDRISSHARED, /*�ӿ������ַ�Ƿ�Ϊ����,ȡֵΪUSP_IF_TRUE,USP_IF_FALSE*/

    SYS_IF_PORTMODE,    /*���ýӿڵ�portģʽ��ȡֵΪUSP_IF_MODEPROMISCUOUS��*/
    SYS_IF_PORTVLANMODE,    /*�ӿ�����vlanʱ��ģʽ*/
    SYS_IF_DTAGENABLE,  /*�Ƿ�ʹ��˫����*/
    SYS_IF_DTAGOUTERTAGSTATUS,
    SYS_IF_PRIORITY,    /*�ӿ�������ȼ�*/
    SYS_IF_DFLTPRIORITY,    /*�ӿ�������ȼ�*/
    SYS_IF_DFLTVID, /*pvid*/
    SYS_IF_INNERPRIORITY,   /*�ӿ�������ȼ�*/
    SYS_IF_INNERVID,    /*pvid*/
    SYS_IF_OUTERTPID,
    SYS_IF_INNERTPID,

    SYS_IF_MTU, /*�ӿڵ�MTU*/
    SYS_IF_QUEUEPOLICY,
    SYS_IF_QUEUESPBMP,
    SYS_IF_QUEUEMAXNUM,
    SYS_IF_QUEUEBWGRAN,
    SYS_IF_QUEUEBWSUPPORT,
    SYS_IF_QUEUEALGSUPPORT,
    SYS_IF_QUEUEMINBWSUPPORT,
    SYS_IF_QUEUEMAXBWSUPPORT,
    SYS_IF_QUEUEWEIGHTSUPPORT,
    SYS_IF_QUEUEPOLICYMODIFYPORTLIST,
    SYS_IF_CONNECTOR,   /*�ӿ�����������*/
    SYS_IF_FLOWCTRL,    /*ȫ/��˫��ģʽ�µ�����,�����combo������Ϊ�ǹ��,��combo�ڹ⡢�����Ч.*/
    SYS_IF_FLOWCTRLCOMBO,   /*ȫ/��˫��ģʽ�µ�����,�����combo������Ϊ�ǵ��,��combo����Ч.*/
    SYS_IF_INACL_CTRL,  /*�뷽��ACLʹ�ܱ�־*/
    SYS_IF_INACL,   /*�뷽��ACL����*/
    SYS_IF_OUTACL_CTRL, /*������ACLʹ�ܱ�־*/
    SYS_IF_OUTACL,  /*������ACL����*/
    SYS_IF_SECURITY,    /*�ӿڰ�ȫ������*/
    SYS_IF_ADDR_LEARNING,   /*�ӿڵ�ַѧϰʹ��*/
    SYS_IF_INBW_CTRL,   /*�뷽���������ʹ��*/
    SYS_IF_INBW,    /*�뷽�������������*/
    SYS_IF_OUTBW_CTRL,  /*�������������ʹ��*/
    SYS_IF_OUTBW,   /*�����������������*/


    SYS_IF_STORMCTRLLEVEL,      /*ȡֵ�μ�USP_STORMCTRL_PERCENT,USP_STORMCTRL_PPS,USP_STORMCTRL_BPS*/
    SYS_IF_STORMCTRLBCASTLIMIT, /*�ӿڹ㲥�����ʿ���,SYS_IF_BCAST_LIMIT*/
    SYS_IF_STORMCTRLBCASTENABLE,    /*�ӿڹ㲥�����ʿ���,SYS_IF_BCAST_CTRL*/
    SYS_IF_STORMCTRLBCASTCBS,   /*�ӿڹ㲥�����ʿ���,SYS_IF_BCAST_CTRL*/
    SYS_IF_STORMCTRLMCASTLIMIT, /*�ӿ��鲥�����ʿ���,SYS_IF_MCAST_LIMIT*/
    SYS_IF_STORMCTRLMCASTENABLE,    /*�ӿ��鲥�����ʿ���,SYS_IF_MCAST_CTRL*/
    SYS_IF_STORMCTRLMCASTCBS,   /*�ӿ��鲥�����ʿ���,SYS_IF_MCAST_CTRL*/
    SYS_IF_STORMCTRLDLFLIMIT,   /*����DLF������,SYS_IF_DLF_LIMIT*/
    SYS_IF_STORMCTRLDLFENABLE,  /*����DLF������,SYS_IF_DLF_CTRL*/
    SYS_IF_STORMCTRLDLFCBS, /*����DLF������,SYS_IF_DLF_CTRL*/
    SYS_IF_STORMCTRLALLLIMIT,   /**/
    SYS_IF_STORMCTRLALLENABLE,  /**/
    SYS_IF_STORMCTRLALLCBS, /**/
    SYS_IF_STORMCTRLUNKNOWNIPMCLIMIT,
    SYS_IF_STORMCTRLUNKNOWNIPMCENABLE,
    SYS_IF_STORMCTRLUNKNOWNIPMCCBS,
    SYS_IF_STORMCTRLKNOWNIPMCLIMIT,
    SYS_IF_STORMCTRLKNOWNIPMCENABLE,
    SYS_IF_STORMCTRLKNOWNIPMCCBS,
    SYS_IF_STORMCTRLUNKNOWNL2MCLIMIT,
    SYS_IF_STORMCTRLUNKNOWNL2MCENABLE,
    SYS_IF_STORMCTRLUNKNOWNL2MCCBS,
    SYS_IF_STORMCTRLKNOWNL2MCLIMIT,
    SYS_IF_STORMCTRLKNOWNL2MCENABLE,
    SYS_IF_STORMCTRLKNOWNL2MCCBS,


    SYS_IF_RER_FLAG,    /*�ӿ�RERʹ�ܱ�־*/
    SYS_IF_BW_GRAN, /*�ӿ������ô�������ȣ���64KbpsΪ��λ*/
    SYS_IF_FLUSHMAC,    /*��պͽӿ���ص�MAC��ַ��������̬����*//*����Ϊoctetstring,��������ΪNULL,�������������ӿ������Ϊvlan����λ,���������vlan�����Ϊ����ӿ�unit��(4�ֽ������ֽ���,��֧�ֶ��)*/
    SYS_IF_FLUSHARP,/*��պͽӿ���ص�ARP��ַ��������̬����*//*����Ϊoctetstring,��������ΪNULL,�������������ӿ������Ϊvlan����λ,���������vlan�����Ϊ����ӿ�unit��(4�ֽ������ֽ���,��֧�ֶ��)*/
    SYS_IF_FLUSHND,         /*����Ϊoctetstring,��������ΪNULL,�������������ӿ������Ϊvlan����λ,���������vlan�����Ϊ����ӿ�unit��(4�ֽ������ֽ���,��֧�ֶ��)*/
    SYS_IF_FLUSHARL,        /*����Ϊoctetstring,��������ΪNULL,�������������ӿ������Ϊvlan����λ,���������vlan�����Ϊ����ӿ�unit��(4�ֽ������ֽ���,��֧�ֶ��)*/
    SYS_IF_FLUSHALL,        /*����Ϊoctetstring,��������ΪNULL,�������������ӿ������Ϊvlan����λ,���������vlan�����Ϊ����ӿ�unit��(4�ֽ������ֽ���,��֧�ֶ��)*/
    SYS_IF_ADD_FILTER,  /*��ӹ��˱�*/
    SYS_IF_DEL_FILER,   /*ɾ�����˱�*/
    SYS_IF_LINK_STATE,/*�ӿ�RER����·״̬*/
    SYS_IF_TXPKT,   /*���¼�����Ϊ64λ�������հ�����*/
    SYS_IF_RXPKT,   /*��������*/
    SYS_IF_TXOCT,   /*�յ���8λ�����*/
    SYS_IF_RXOCT,   /*����8λ�����*/
    SYS_IF_FRAG,    /*��Ƭ������*/
    SYS_IF_JUMBO,   /*jumbo֡����*/
    SYS_IF_CRC, /*�հ�CRC���������*/
    SYS_IF_COLLS,   /*��ײ������*/
    SYS_IF_USIZE,   /*���̰�����*/
    SYS_IF_OSIZE,   /*������*/
    SYS_IF_TX64OCTET,
    SYS_IF_TX65TO127OCTET,
    SYS_IF_TX128TO255OCTET,
    SYS_IF_TX256TO511OCTET,
    SYS_IF_TX512TO1023OCTET,
    SYS_IF_TX1024TO1518OCTET,
    SYS_IF_TX1519TOMaxOCTET,
    SYS_IF_RX64OCTET,
    SYS_IF_RX65TO127OCTET,
    SYS_IF_RX128TO255OCTET,
    SYS_IF_RX256TO511OCTET,
    SYS_IF_RX512TO1023OCTET,
    SYS_IF_RX1024TO1518OCTET,
    SYS_IF_RX1519TOMaxOCTET,
    SYS_IF_SYMBOL,
    SYS_IF_ERRORSYMBOL,
    SYS_IF_TOTALRXPKT,      /*������ȷ�����İ�����*/
    SYS_IF_TOTALTXPKT,      /*������ȷ�����İ�����*/
    SYS_IF_RXUCAST, /*�յ��ĵ���������*/
    SYS_IF_RXMCAST, /*�յ����鲥������*/
    SYS_IF_RXBCAST, /*�յ��Ĺ㲥������*/
    SYS_IF_TXUCAST, /*���͵ĵ���������*/
    SYS_IF_TXMCAST, /*���͵��鲥������*/
    SYS_IF_TXBCAST, /*���͵Ĺ㲥������*/
    SYS_IF_RXPAUSEPKT,
    SYS_IF_TXPAUSEPKT,
    SYS_IF_ALIGNMENTERRORS,     /*A count of frames received on a particular
                                interface that are not an integral number of
                                octets in length and do not pass the FCS checK.
                                �հ�AlignmentErrors������,EtherLike-MIB*/
    SYS_IF_SINGLECOLLISIONFRAMES,   /*A count of successfully transmitted frames on
                                    a particular interface for which transmission
                                    is inhibited by exactly one collision,EtherLike-MIB*/

    SYS_IF_MULTIPLECOLLISIONFRAMES, /*A count of successfully transmitted frames on
                                    a particular interface for which transmission
                                     is inhibited by more than one collision,EtherLike-MIB*/
    SYS_IF_SQETESTERRORS,       /*A count of times that the SQE TEST ERROR
                                    message is generated by the PLS sublayer for a
                                    particular interface. The SQE TEST ERROR
                                    message is defined in section 7.2.2.2.4 of
                                    ANSI/IEEE 802.3-1985 and its generation is
                                    described in section 7.2.4.6 of the same
                                    document,EtherLike-MIB*/
    SYS_IF_DEFERREDTRANSMISSIONS,       /*A count of frames for which the first
                                transmission attempt on a particular interface
                                is delayed because the medium is busy,EtherLike-MIB*/

    SYS_IF_LATECOLLISIONS,      /*The number of times that a collision is
                                detected on a particular interface later than
                                512 bit-times into the transmission of a
                                packet,EtherLike-MIB*/

    SYS_IF_EXCESSIVECOLLISIONS,     /*A count of frames for which transmission on a
                                particular interface fails due to excessive
                                collisions,EtherLike-MIB*/

    SYS_IF_INTERNALMACTRANSMITERRORS,       /*A count of frames for which transmission on a
                                        particular interface fails due to an internal
                                        MAC sublayer transmit error. A frame is only
                                        counted by an instance of this object if it is
                                        not counted by the corresponding instance of
                                        either the dot3StatsLateCollisions object, the
                                        dot3StatsExcessiveCollisions object, or the
                                        dot3StatsCarrierSenseErrors object,EtherLike-MIB*/

    SYS_IF_CARRIERSENSEERRORS,      /*The number of times that the carrier sense
                                    condition was lost or never asserted when
                                    attempting to transmit a frame on a particular
                                    interface,EtherLike-MIB*/

    SYS_IF_FRAMETOOLONGS,       /*A count of frames received on a particular
                                interface that exceed the maximum permitted
                                frame size,EtherLike-MIB*/

    SYS_IF_INTERNALMACRECEIVEERRORS,        /*A count of frames for which reception on a
                                        particular interface fails due to an internal
                                        MAC sublayer receive error. A frame is only
                                        counted by an instance of this object if it is
                                        not counted by the corresponding instance of
                                        either the dot3StatsFrameTooLongs object, the
                                        dot3StatsAlignmentErrors object, or the
                                        dot3StatsFCSErrors object,EtherLike-MIB*/


    SYS_IF_FILTER_OUT_CTRL,
    SYS_IF_FILTER_IN_CTRL,
    SYS_IF_FILTER_IN_GROUP,
    SYS_IF_FILTER_OUT_GROUP,
    SYS_IF_FLOWCTRL_STATUS,

    SYS_IF_MDIX_SUPPORT,
    SYS_IF_MDIX_STATUS,

    SYS_IF_TRUNK_STATE,     /*�Է�TRUNK�ӿڵ���Ч��־λ����*/
    SYS_IF_TRUNK_ID,        /*�Է�TRUNK�ӿ�����trunkID*/

    SYS_IF_MIRROR_INGRESS,  /*��д�˿ڵľ���״̬|enable|disable*/
    SYS_IF_MIRROR_EGRESS,   /*��д�˿ڵľ���״̬|enable|disable*/
    SYS_IF_MIRRORTO_PORT,   /*��д�˿ڵľ���Ŀ�Ķ˿�*/
    SYS_IF_INBANDNM_ENABLE,    /*�ӿ���������״̬*/
    SYS_IF_MCASTFILTER_MODE, /* config for interface mcast addr filter */
    SYS_IF_INPACKETCOUNTER300S,  /* SET/GET Input Packet counter during 5 minutes */
    SYS_IF_OUTPACKETCOUNTER300S, /* SET/GET Output Packet counter during 5 minutes */
    SYS_IF_INOCTETCOUNTER300S,  /* SET/GET Input Byte counter during 5 minutes */
    SYS_IF_OUTOCTETCOUNTER300S, /* SET/GET Output Byte counter during 5 minutes */
    SYS_IF_BACKBYBACKSTATUS,
    SYS_IF_PKTFILTERCOUNTER,
    SYS_IF_RESESTATS,
    SYS_IF_TDM,
    SYS_IF_BPDU_STATUS,
    SYS_IF_STP_STATUS,

    SYS_IF_MACLIMITENABLE,
    SYS_IF_MAC_LIMIT,       /*ע��:оƬ������˿ڣ�trunk�˿ڣ�vlan��֧�����*/
    SYS_IF_MACLIMIT,        /*ע��:оƬ������˿ڣ�trunk�˿ڣ�vlan��֧�����*/
    SYS_IF_MACLIMITOVERDROP,    /*ע��:оƬ������˿ڣ�trunk�˿ڣ�vlan��֧�����*/
    SYS_IF_MACLIMITOVERTOCPU,   /*ע��:оƬ������˿ڣ�trunk�˿ڣ�vlan��֧�����.���ڲ�ʹ��*/
    SYS_IF_MACLIMITALARMENABLE, /*����mac limit num�ĸ澯����*/
    SYS_IF_MACCOUNT,    /*ע��:оƬ������˿ڣ�trunk�˿ڣ�vlan��֧�����*/
    SYS_IF_MACSTATICCOUNT,      /*��̬mac��ַ����*/
    SYS_IF_MACDYNAMICCOUNT,     /*��̬mac��ַ����*/
    SYS_IF_MACBLACKHOLECOUNT,   /*�ڶ�mac��ַ����*/

    SYS_IF_SECURITYENABLE,
    SYS_IF_SECURITYMACLIMITNUM,
    SYS_IF_SECURITYVIOLATIONMODE,
    SYS_IF_SECURITYSTICKYMACLEARNINGENABLE,

    SYS_IF_INBANDNM_VID,
    SYS_IF_LINKMONITOR,

    SYS_IF_WORKMODE,        /*���ýӿ��ǽ�����ģʽ����·����ģʽ*/
    SYS_IF_IPADD,/*add phy ip address, 2008-1-2*/
    SYS_IF_IPDEL,/*delete phy ip address, 2008-1-2*/
    SYS_IF_IP6ADD,
    SYS_IF_IP6DEL,
    SYS_IF_IPADDR,
    SYS_IF_IPV4ADDR,
    SYS_IF_IPV6ADDR,
    SYS_IF_IPADDREX,
    SYS_IF_IPV4ADDREX,
    SYS_IF_IPV6ADDREX,
    SYS_IF_IPADDRNUM,
    SYS_IF_IPV4ADDRNUM,
    SYS_IF_IPV6ADDRNUM,
    SYS_IF_IPV6ENABLE,          /*�ӿ���ʹ��IPv6����,ipv6 enable*/
    SYS_IF_IPV6AUTOLINKLOCAL,/*�����Ƿ��Զ�������·���ص�ַ,ipv6 address auto link-local*/
    SYS_IF_FILTER_STATS_GET,
    SYS_IF_FILTER_STATS_RESET,
    SYS_IF_FILTER_STATS_TYPE,
    SYS_IF_FILTER_STATS_TOTAL_GET,
    SYS_IF_XVLAN_ENABLE,
    SYS_IF_XVLAN_MISSACT,
    SYS_IF_LACP_ENABLE,             /*not used*/
    SYS_IF_LINKFORCE,
    SYS_IF_M2TYPE,
    SYS_IF_M2SPEED,
    SYS_IF_OUTDISCARDS,
    SYS_IF_OUTNUCASTPKTS,
    SYS_IF_INUNKNOWNPROTOS,
    SYS_IF_INERRORS,
    SYS_IF_INDISCARDS,
    SYS_IF_INNUCASTPKTS,
    SYS_IF_LASTCHANGE,
    SYS_IF_OUTERRORS,
    SYS_IF_OUTQLEN,
    SYS_IF_SPECIFIC,
    SYS_IF_LINKTRAPENABLE,
    SYS_IF_PROMISCUOUSMODE,
    SYS_IF_CONNECTORPRESENT,    /*rfc2233*/
    SYS_IF_COUNTERDISCONTINUITYTIME,    /*rfc2233*/

    SYS_IF_COMBOPORT_SUPPORT,
    SYS_IF_COMBOPORT_TYPE,

    /*
    ��������������Ϊһ�飬������SYS_IF_HIGHSPEED,SYS_IF_DUPLEX,SYS_IF_AUTONEGO����
    */
    SYS_IF_DUPLEXSPEEDCAP,
    SYS_IF_DUPLEXSPEED,
    SYS_IF_CFGDUPLEXSPEED,

    SYS_IF_STORMRATE_GRAN,
    SYS_IF_AUTOSTATEEXCLUDE,    /*�����Զ�״̬�ų�*/

    SYS_IF_STATICTAGGEDVLAN,    /*���úͻ�ȡ����ӿھ�̬tag vlan*/
    SYS_IF_STATICUNTAGGEDVLAN,  /*���úͻ�ȡ����ӿھ�̬untag vlan*/
    SYS_IF_DYNAMICTAGGEDVLAN,   /*���úͻ�ȡ����ӿڶ�̬tag vlan*/
    SYS_IF_DYNAMICUNTAGGEDVLAN, /*���úͻ�ȡ����ӿڶ�̬untag vlan*/

    SYS_IF_ALLVLAN,         /*ȡ���ӿ������õ�ȫ��vlan*/
    SYS_IF_STATICVLAN,      /*ȡ���ӿ������õ�ȫ����̬vlan*/
    SYS_IF_DYNAMICVLAN, /*ȡ���ӿ������õ�ȫ����̬vlan*/
    SYS_IF_TAGGEDVLAN,      /*ȡ���ӿ������õ�ȫ��tagged vlan*/

    SYS_IF_ALLTYPEVLAN,         /*ȡ���ӿ������õ�ȫ��vlan������tag/untag����*/
    SYS_IF_STATICTYPEVLAN,      /*ȡ���ӿ������õ�ȫ����̬vlan������tag/untag����*/
    SYS_IF_DYNAMICTYPEVLAN, /*ȡ���ӿ������õ�ȫ����̬vlan������tag/untag����*/

    SYS_IF_REMOVEVLAN,

    SYS_IF_PVLANADDVLAN,    /*���ݲ�ͬ�ӿ��������pvlan��primary��secondary vlan*/
    SYS_IF_PVLANDELVLAN,    /*���ݲ�ͬ�ӿ�����ɾ��pvlan��primary��secondary vlan*/
    SYS_IF_PVLANGETVLAN,

    SYS_IF_TSTVLAN,
    SYS_IF_TSTSTATICVLAN,
    SYS_IF_TSTDYNAMICVLAN,
    SYS_IF_TSTTAGGEDVLAN,
    SYS_IF_TSTUNTAGGEDVLAN,
    SYS_IF_TSTSTATICTAGGEDVLAN,
    SYS_IF_TSTSTATICUNTAGGEDVLAN,
    SYS_IF_TSTDYNAMICTAGGEDVLAN,
    SYS_IF_TSTDYNAMICUNTAGGEDVLAN,

    SYS_IF_MACLEARNINGENABLE,   /*���ö˿�mac��ַѧϰģʽ,P242,P297,ȡֵΪUSP_TRUE��USP_FALSE*/
    SYS_IF_MACLEARNINGCLASS,    /*���ö˿�mac��ַѧϰ���ȼ�������վ���ƶ�*/
    SYS_IF_MACCLASSBASEDSMENABLE,       /*����������ͬ���ȼ��Ľӿڷ���MAC ��ַƯ��,ȡֵΪUSP_TRUE��USP_FALSE.CLASS BASED STATION MOVEMENT*/

    SYS_IF_VLANPRECEDENCE,
    SYS_IF_MACVLANENABLE,   /*ip mac vlan*/
    SYS_IF_MACVLANTRIGGERENABLE,
    SYS_IF_IPVLANENABLE,    /*ip subnet vlan*/

    SYS_IF_URPFMODE,    /*urpf mode*/
    SYS_IF_URPFDFLTROUTECHECK,  /*urpf mode*/

    SYS_IF_DROPTAGGEDPKT,       /*drops all tagged packets*/
    SYS_IF_DROPUNTAGGEDPKT, /*drops all untagged packets*/

    SYS_IF_ROWSTATUS,
    SYS_IF_EXIST,
    SYS_IF_CREATIONTIME,
    SYS_IF_SLFTOCPU,    /*���Ҳ���Դmac����cpu�ļ���ֵ,����mac_limit��صĶ˿ڰ�ȫ*/

    SYS_IF_ARPSTRICTLEARNMODE,  /*����ip�ӿڵ�arp�ϸ�ѧϰ*/
    SYS_IF_ARPLIMITNUM,         /*����ip�ӿڵ�arpѧϰ��Ŀ*/
    SYS_IF_ARPNUM,              /*get ,ip�ӿ���ѧϰ����ȫ��arp��Ŀ*/
    SYS_IF_DYNAMICARPNUM,       /*get ,ip�ӿ���ѧϰ����arp��Ŀ*/
    SYS_IF_DHCPTRIGGEREDARPNUM, /*dhcp triggerѧϰ����arp������Ŀ,uint32_t*/
    SYS_IF_STATICARPNUM,        /*��̬arp������Ŀ,uint32_t*/
    SYS_IF_ARPNUMRESET, /*arp������0*/
    SYS_IF_ARPPROXYENABLE,      /*���ڿ���ip�ӿ�arp proxy�Ƿ����*/
    SYS_IF_ARPLEARNDHCPTRIGGER, /*����dhcp����ѧϰarp����*/

    SYS_IF_NDSTRICTLEARNMODE,   /*����ip�ӿڵ�ND�ϸ�ѧϰ*/
    SYS_IF_NDLIMITNUM,          /*����ip�ӿڵ�NDѧϰ��Ŀ*/
    SYS_IF_NDNUM,       /*get ,ip�ӿ���ѧϰ����ND��Ŀ*/
    SYS_IF_DYNAMICNDNUM,        /*get ,ip�ӿ���ѧϰ����ND��Ŀ*/
    SYS_IF_STATICNDNUM,     /*��̬ND������Ŀ,uint32_t*/
    SYS_IF_NDNUMRESET,  /*Nd������0*/
    SYS_IF_NDPROXYENABLE,


    SYS_IF_REGSENDFUNC, /*ע�ᷢ������*/
    SYS_IF_REGRECVFUNC, /*ע���հ�����*/
    SYS_IF_REGNOTIFYFUNC, /*ע��end�ڲ���Ϣͨ�溯��*/
    SYS_IF_REGINPUTFILTERFUNC,  /*ע���հ����˺���*/
    SYS_IF_REGOUTPUTFILTERFUNC, /*ע�ᷢ�����˺���*/


    /*
    �ϲ����ģ���Ӧ��Э��ֱ�ӵ���hwApiGetApi/hwApiSetApi����Ӧ����
    trunk�ӿڵ�Ӧ��ʱ�������⡣����ϲ����ģ���Ӧ��Э��
    ��Ҫֱ�ӵ���hwApiGetApi/hwApiSetApi���������ǵ���ͳһ��uspIfSetApi/uspIfGetApi
    ����������������ɶ�trunk�ӿڵĴ�����������ΪtHwApiFunc��
    */
    SYS_IF_HWAPIFUNC,

    /*
    ���������������SYS_IF_HWAPIFUNC�����ּ�����ͬ������������ʵ�ֵ������ֲ�ͬ��
    USP_CMD_COMMIT��USP_CMD_SET��
    USP_CMD_COMMIT��ָ�Ĵ��ģ��������ϲ��trunk����ͬ����
    USP_CMD_SET�����ϲ�trunk����ͬ����
    */
    SYS_IF_HWAPIFUNCEX,

    SYS_IF_SYNCAPIFUNC,

    /*
    �ϲ����ģ���Ӧ��Э����Ҫ֪��ĳЩ������agֱ��֧�ֻ���phy֧�֣�
    Ȼ���ϲ�����ٸ���֧���������SYS_IF_HWAPIFUNC�����ֵĶ���������
    ��������ΪtHwApiFunc���п��ܲ�ʹ��arg������HwIfGet����ʵ����Ҫ��дaction�ֶΡ�
    */
    SYS_IF_HWAPIFUNCSUPPORT,

    /*
    �������������ڴ�ȡЭ��ʹ�ܱ�־��
    ���ڽ�������������ֻ��Ҫ��vlan�ӿڽ��������Ա�֤vlan
    ��ӡ�ɾ������ӿ�ʱ��Ӳ����cpu������ͬ����
    ���ڶ���Э��Ҳ���Ե��ô˽ӿڡ�
    */

    //SYS_IF_PROTOSTATE,                /*����������Ϊoctetstring*,*/
    SYS_IF_PROTOENABLE,             /*����������Ϊint32_t*,*/

    SYS_IF_PROTODISABLE,            /*����������Ϊint32_t*,*/
    SYS_IF_PROTOCAPABILITYINCLUDE,/*�ӿ�֧�ֵĶ�������Э��bitlist������,���ģ�鲻������hw��ʵʱ��ȡ�����ģ������ܸ��ݸ������ֽ��нӿ��Ƿ���ʹ��ָ��Э����жϡ�ȡֵ�μ�uspDfs.h*/
    SYS_IF_PROTOCAPABILITYEXCLUDE,/*�ӿڲ�֧�ֵĶ�������Э��bitlist������,���ģ�鲻������hw��ʵʱ��ȡ�����ģ������ܸ��ݸ������ֽ��нӿ��Ƿ���ʹ��ָ��Э����жϡ�ȡֵ�μ�uspDfs.h*/


    /*laser DDM*/
    SYS_IF_LASERDDM,
    SYS_IF_LASERWARNINGS,
    SYS_IF_LASERTRAPENABLE,
    SYS_IF_LASERSUPPORTFUNCS,
    SYS_IF_LASERTEMPERATURE,
    SYS_IF_LASERTEMPERATURELOWLMT,
    SYS_IF_LASERTEMPERATUREHIGHLMT,
    SYS_IF_LASERRXPOWER,
    SYS_IF_LASERRXPOWERLOWLMT,
    SYS_IF_LASERRXPOWERHIGHLMT,
    SYS_IF_LASERTXPOWER,
    SYS_IF_LASERTXPOWERLOWLMT,
    SYS_IF_LASERTXPOWERHIGHLMT,
    SYS_IF_LASERVOLTAGE,
    SYS_IF_LASERVOLTAGELOWLMT,
    SYS_IF_LASERVOLTAGEHIGHLMT,
    SYS_IF_LASERBIASCURRENT,
    SYS_IF_LASERBIASCURRENTLOWLMT,
    SYS_IF_LASERBIASCURRENTHIGHLMT,
    SYS_IF_LASERRXPOWERAUTOLMT,
    SYS_IF_LASERTXPOWERAUTOLMT,
    SYS_IF_LASERTEMPERATUREAUTOLMT,
    SYS_IF_LASERBIASCURRENTAUTOLMT,
    SYS_IF_LASERVOLTAGEAUTOLMT,
    SYS_IF_LASERTXDISABLE,
    SYS_IF_LASERRATESELECT,
    SYS_IF_LASERTXLOS,
    SYS_IF_LASERTXFAULT,
    SYS_IF_LASERMODULEPRESENT,
    SYS_IF_LASERWAVELENGTH,
    SYS_IF_LASERLINKLENGTH,
    SYS_IF_LASERIDENTIFIER,
    SYS_IF_LASERTRANSCEIVER,
    SYS_IF_LASERENCODING,
    SYS_IF_LASERCONNECTOR,
    SYS_IF_LASERRATEIDENTIFIER,
    SYS_IF_LASERDMSUPPORT,
    SYS_IF_LASERCALIBRATEDTYPE,
    SYS_IF_LASERVENDORNAME,
    SYS_IF_LASERVENDORPN,
    SYS_IF_LASERVENDORSN,
    SYS_IF_LASERVENDORREV,
    SYS_IF_LASERDATECODE,
    SYS_IF_LASERNOMINALBR,
    SYS_IF_LASERMINBR,
    SYS_IF_LASERMAXBR,
#if 0   /*Ϊ�˲�����ԭ��������˳��,�ƶ�����*/
    SYS_IF_LASERPEC,        /*octetstring*/
    SYS_IF_LASERCLEI,       /*octetstring*/
    SYS_IF_LASERUNSUPPORTEDTRANSCEIVER,
    SYS_IF_LASERERRDISABLEDETECT,
    SYS_IF_LASERALS,                        /*ȡֵΪUSP_ENABLE,USP_DISABLE,USP_ENABLE��ʾ֧�ֹ�ģ���Զ��ض�*/
#endif

    SYS_IF_VIRTUALID,   /*ȡ��������ӿڵ�ID����vid,tunnelid��*/
    SYS_IF_INDEX,       /*ȡ�ӿڵ�ifIndex,��rfc1213�е�ifIndex*/
    SYS_IF_DOT1DPORTID, /*ȡ�ӿڵ�dot1dbaseport,��rfc14933�е�dot1dBasePort����ͬ���ڲ���index��*/

    SYS_IF_BCASTBLOCKENABLE,
    SYS_IF_UNKNOWNUCASTBLOCKENABLE,
    SYS_IF_UNKNOWNMCASTBLOCKENABLE,

    SYS_IF_PORTISOLATEENABLE,   /*�Ƿ�ʹ�ܶ˿ڸ��빦�ܣ������ڲ����ø����������*/
    SYS_IF_PORTISOLATEGROUP,    /*���ö˿ڸ����飬���������ø����������*/

    SYS_IF_PORTTYPE,    /*���ýӿ���nni����uni����*/

    SYS_IF_QOSTRUST,        /*get & set, ����int *,ȡֵ�μ�USP_IF_TRUSTDSCP��*/
    SYS_IF_QOSTRUSTDOT1P,       /*get & set, ����int *,ȡֵ�μ�USP_IF_ENABLE�ȡ�����������uspIfQosTrust�����ظ������ڲ�ͬ����������ơ�оƬ�ϸüĴ����Ƕ����ģ�����ݱ�����*/
    SYS_IF_QOSTRUSTDSCP,        /*get & set, ����int *,ȡֵ�μ�USP_IF_ENABLE�ȡ�����������uspIfQosTrust�����ظ������ڲ�ͬ����������ơ�оƬ�ϸüĴ����Ƕ����ģ�����ݱ�����*/
    SYS_IF_DIFFSRVPROFILE,  /*get & set, ����int *,ȡֵ�μ�differservice profile id*/
    SYS_IF_DIFFSRVPINGRESSROFILE,       /*get & set, ����int *,ȡֵ�μ�differservice profile id*/
    SYS_IF_DIFFSRVPEGRESSROFILE,        /*get & set, ����int *,ȡֵ�μ�differservice profile id*/

    SYS_IF_IPMCV4FWDMODE,           /*vlan�鲥ת��ģʽ����,����int* ,ȡֵ�μ�USP_IPMC_L2FWD*/
    SYS_IF_IPMCV6FWDMODE,           /*vlan�鲥ת��ģʽ����,����int* ,ȡֵ�μ�USP_IPMC_L2FWD*/
    SYS_IF_UNKNOWNIPMCV4TOCPUENABLE,    /*δ֪IPMC�鲥��cpuʹ�ܣ�����Ϊint*,USP_IF_TRUE */
    SYS_IF_UNKNOWNIPMCV6TOCPUENABLE,    /*δ֪IPMC�鲥��cpuʹ�ܣ�����Ϊint*,USP_IF_TRUE */

    SYS_IF_INGRESSFILTER,   /*rfc2674q,dot1qPortIngressFiltering*/
    SYS_IF_ACCEPTFRAME,     /*rfc2674q,dot1qPortAcceptableFrameTypes*/
    SYS_IF_DOT1DCAPABILITIES,       /*rfc2674q,dot1dPortCapabilities*/

    SYS_IF_VRFID,
    SYS_IF_VPNNAME,

    SYS_IF_NETFLOWTOCPU,    /*only get, netflow��cpu�İ�����*/

    SYS_IF_ICMPV4REDIRECTENABLE,
    SYS_IF_ICMPV6REDIRECTENABLE,
    SYS_IF_DLIPENABLE,      /*DLP enable, int *,ȡֵΪUSP_IF_TRUE*/

    SYS_IF_MFFMODE,     /*MAC-Forced forwarding(MFF),USP_IF_UNI,USP_IF_NNI*/

    SYS_IF_ENCAPTYPE,       /*���ýӿڷ�װģʽ,ȡֵ�μ�USP_ENCAP_ETHERNET*/
    SYS_IF_IFGBYTES,        /*���ýӿ�֡����ֽ���,int32_t,default is 0*/


    SYS_IF_RXMPCPPKT,
    SYS_IF_RXMPCPOCTET,
    SYS_IF_RXREPORTPKT,
    SYS_IF_RXREPORTABORTPKT,
    SYS_IF_RXOAMPDUPKT,
    SYS_IF_RXOAMPOCTET,
    SYS_IF_RXLLIDERRPKT,
    SYS_IF_RXDROPUNGRANTEDPKT,
    SYS_IF_RXREGISTERREQPKT,
    SYS_IF_RXREGISTERACKPKT,
    SYS_IF_TXMPCPPKT,
    SYS_IF_TXMPCPOCTET,
    SYS_IF_TXGATEPKT,
    SYS_IF_TXOAMPDUPKT,
    SYS_IF_TXOAMPOCTET,
    SYS_IF_TXREGISTERREQPKT,
    SYS_IF_TXREGISTERACKPKT,
    SYS_IF_TXREPORTPKT,

    SYS_IF_TRUNKPRIMARYPORT,
    SYS_IF_TRUNKSECONDARYPORT,
    SYS_IF_TRUNKWORKPORT,
    SYS_IF_TRUNKFORCESWITCHENABLE,      /*������ǿ�Ƶ���,int32_t,USP_IF_ENABLE,USP_IF_DISABLE*/
    SYS_IF_TRUNKFORCESWITCHPORT,        /*������ǿ�Ƶ������˿�,Uint32_t*/
    SYS_IF_TRUNKMANUALSWITCHENABLE,     /*�������ֹ�����,int32_t,USP_IF_ENABLE,USP_IF_DISABLE*/
    SYS_IF_TRUNKMANUALSWITCHPORT,       /*�������ֹ��������˿�,Uint32_t*/
    SYS_IF_TRUNKLOCKENABLE,             /*����������,int32_t,USP_IF_ENABLE,USP_IF_DISABLE*/
    SYS_IF_TRUNKREVERT,                 /*get & set,�������þۺ϶˿��Ƿ����*/
    SYS_IF_TRUNKWTR,                    /*get & set,�������þۺ϶˿ڻ���ʱ��*/

    SYS_IF_ISNPTEND,                        /*�ж�ָ���ӿ��Ƿ�Ϊnpt end����,����Ϊint32_t*/
    SYS_IF_PMTUDENABLE,                 /*pmtu �Ƿ�ʹ�ܱ��,ȡֵΪUSP_IF_TRUE,USP_IF_FALSE*/

    SYS_IF_MSTATS,                      /*����Ϊ�ṹtHwPortStatsEntry*/
    SYS_IF_VCTSUPPORT,                  /*�Ƿ�֧��vct���ϼ��,uint32_t,true/false*/
    SYS_IF_VCTMODE,                     /*vct���ϼ��ģʽ*/
    SYS_IF_VCTENABLE,                   /*ʹ��vct���ϼ��,uint32_t,true/false*/
    SYS_IF_VCTSTATE,                    /*vct���ϼ����*/
    SYS_IF_VCTFAULTDISTANCE,            /*vct���Ͼ���,uint32_t,unit meter*/

    SYS_IF_DOWNHOLDOFFTIME,             /*default is 0,50-5000ms,�˿�down�¼���ʱ�ϱ�ʱ��,����ĳЩ��Ҫ�ӳ��ϱ�down�¼��ĳ���:�����ϻ�ĳЩ�����豸�ı�������*/

    SYS_IF_DOT3AHLBDROPSTATSPKTS,       /*get����dot3ah����ʹ�ܶ����İ�����,����Ϊuint64b_t*/
    SYS_IF_DOT3AHLBDROPSTATSENABLE,         /*get/set dot3ah����ʹ��/ȥʹ�ܶ����İ���������,int32_t, ȡֵUSP_IF_TRUE,USP_IF_FALSE*/

    SYS_IF_PARENTPORT,                  /*uint32_t*,��ȡ���ӿ�unit��*/

    SYS_IF_LASERPEC,        /*octetstring,only get*/
    SYS_IF_LASERCLEI,       /*octetstring,only get*/
    SYS_IF_LASERUSI,        /*octetstring,only get*/

    SYS_IF_DFLTADMINSTATUS,     /*int32_t*,�ӿ�ȱʡ����״̬,only get*/
    SYS_IF_MAUDFLTDFLTTYPE,
    SYS_IF_MAUDFLTDFLTTYPECOMBO,

    SYS_IF_MSTATSEX,                        /*����Ϊoctetstring,octetstring->pBufָ��ṹtHwPortStatsEntry.
                                            ����������SYS_IF_MSTATS�����ֵ��������ڲ�����һ��,�������ֿ�����
                                            �ѵ�����.��nmApi����Լ��һ��.
                                            */

    SYS_IF_LASERUNSUPPORTEDTRANSCEIVER,     /*ȡֵΪUSP_ENABLE,USP_DISABLE,USP_ENABLE��ʾ֧�ֲ�֧�ֹ�ģ��*/
    SYS_IF_LASERERRDISABLEDETECT,           /*ȡֵΪUSP_TRUE,VOS_FALSE,USP_TRUE��ʾ�������*/
    SYS_IF_LASERALS,                        /*ȡֵΪUSP_ENABLE,USP_DISABLE,USP_ENABLE��ʾ֧�ֹ�ģ���Զ��ض�*/

    /*trunk cmd*/
    SYS_IF_TRUNKCREATE=4000,        /*����һ��TRUNK��*/
    SYS_IF_TRUNKDELETE,     /*ɾ��һ��TRUNK��*/
    SYS_IF_TRUNKMODE,       /*�޸�trunk�Ķ˿ڼ���ģʽ.ȡֵΪHWAPI_AGGMODE_MANUAL,HWAPI_AGGMODE_STATICLACP��ֵ*/
    SYS_IF_TRUNKADDPORT,    /*��trunk�˿ڼ�������˿�*/
    SYS_IF_TRUNKDELPORT,        /*��TRUNKɾ����Ա�˿�*/
    SYS_IF_TRUNKMEMPORT,        /*��TRUNK��ȡ��Ա�˿�*/
    SYS_IF_TRUNKRTAG,       /*TRUNK�ӿڵķ����㷨*/
    SYS_IF_TRUNKNONUCASTRTAG,   /*get & set,�μ�NONUCAST_TRUNK_BLOCK_MASK,P634,For broadcast, L2 multicast, DLF: {SA MAC, DA MAC, Ingress Module ID/Port or Ingress TGID}*/
    SYS_IF_TRUNKRTAGPROFILE,    /*trunk�����㷨��ģ��ID*/
    SYS_IF_TRUNKBASEINDEX,
    SYS_IF_TRUNKMAXMEMBERPORTNUM,
    SYS_IF_TRUNKACTIVEMEMBERPORTNUM,
    SYS_IF_TRUNKLEASEACTIVEMEMBERPORTNUM,
    SYS_IF_TRUNKUNKNOWNUCASTRTAG,   /*get & set , int * ,δ֪���������㷨*/
    SYS_IF_TRUNKUNKNOWNUCASTRTAGPROFILE,    /*get & set , int * ,trunk�����㷨��ģ��ID*/
    SYS_IF_LACPSELECTPORT,          /*����Ϊunit������ָ��*/
    SYS_IF_LACPUNSELECTPORT,        /*����Ϊunit������ָ��*/




    /*pos cmd*/
    SYS_IF_POSDFLTMEDIUMTYPE=4200,
    SYS_IF_POSMEDIUMTYPE,
    SYS_IF_POSAUGMAPPING,
    SYS_IF_POSLOOPBACK,
    SYS_IF_POSCLOCK,
    SYS_IF_POSCRC,
    SYS_IF_POSSCRAMBLE,
    SYS_IF_POSC2,
    SYS_IF_POSJ0,
    SYS_IF_POSJ1,
    SYS_IF_POSSDTHRSHLD,
    SYS_IF_POSSFTHRSHLD,
    SYS_IF_POSJ0MODE,       /*ȡֵΪ1,16,64*/
    SYS_IF_POSJ1MODE,       /*ȡֵΪ1,16,64*/

    SYS_IF_POSISECPOS,      /*USP_IF_TRUE/USP_IF_FALSE,���ڱ�ʾ�Ƿ�Ϊecpos��*/
    SYS_IF_POSCHANNELIZATION,
    SYS_IF_POSADDAU4POSCHANNEL,
    SYS_IF_POSDELAU4POSCHANNEL,
    SYS_IF_POSADDAU4DS3CHANNEL,
    SYS_IF_POSDELAU4DS3CHANNEL,
    SYS_IF_POSADDAU3POSCHANNEL,
    SYS_IF_POSDELAU3POSCHANNEL,
    SYS_IF_POSADDAU3DS3CHANNEL,
    SYS_IF_POSDELAU3DS3CHANNEL,
    SYS_IF_POSADDSTS1POSCHANNEL,
    SYS_IF_POSDELSTS1POSCHANNEL,
    SYS_IF_POSADDSTS1DS3CHANNEL,
    SYS_IF_POSDELSTS1DS3CHANNEL,
    SYS_IF_POSDELAU3CHANNEL,

    SYS_IF_POSAU4POSCHANNELS,
    SYS_IF_POSAU4DS3CHANNELS,
    SYS_IF_POSAU3POSCHANNELS,
    SYS_IF_POSSTS1POSCHANNELS,
    SYS_IF_POSAU3DS3CHANNELS,
    SYS_IF_POSSTS1DS3CHANNELS,

    SYS_IF_POSUSEDCHANNELS,     /*octetstring, bitListSet/Tst*/
    SYS_IF_POSADDCHANNEL,   /*
                                ����Ϊoctetstring,
                                �����ṹ����:
                                1 bytes          1 bytes      1 btyes
                                channelnumber    linetype     channelization
                                */
    SYS_IF_POSDELCHANNEL,   /*
                                ����Ϊoctetstring,
                                �����ṹ����:
                                1 bytes          1 bytes      1 btyes
                                channelnumber    linetype     channelization
                                */
    SYS_IF_POSREMOVEALLCHANNELS,
    SYS_IF_POSALLCHANNELS,  /*octetstring,1byte channel, 1byte linetype, 1byte channelization, 4byte ifindex*/


    SYS_IF_POSSECTIONCURRENTSTATUS,     /*uint32_t,
                                            The various bit positions are:
                                            1   sonetSectionNoDefect
                                            2   sonetSectionLOS
                                            4   sonetSectionLOF*/
    SYS_IF_POSSECTIONCURRENTESs,
    SYS_IF_POSSECTIONCURRENTSESs,
    SYS_IF_POSSECTIONCURRENTSEFSs,
    SYS_IF_POSSECTIONCURRENTCVs,

    SYS_IF_POSLINECURRENTSTATUS,        /*uint32_t,
                                            The various bit positions are:
                                           1   sonetLineNoDefect
                                           2   sonetLineAIS
                                           4   sonetLineRDI*/
    SYS_IF_POSLINECURRENTESs,
    SYS_IF_POSLINECURRENTSESs,
    SYS_IF_POSLINECURRENTCVs,
    SYS_IF_POSLINECURRENTUASs,

    SYS_IF_POSFARENDLINECURRENTESs,
    SYS_IF_POSFARENDLINECURRENTSESs,
    SYS_IF_POSFARENDLINECURRENTCVs,
    SYS_IF_POSFARENDLINECURRENTUASs,


    /*tdm cmd*/
    SYS_IF_TDMFRAMER=4800,
    SYS_IF_TDMPKTFRAMER,
    SYS_IF_TDMLIULOOPBACK,
    SYS_IF_TDMLIUTAOS,
    SYS_IF_TDMLIUBITSCLOCK,
    SYS_IF_TDMLIURXPERFORMANCEMON,
    SYS_IF_TDMLIUTXPERFORMANCEMON,
    SYS_IF_TDMLIUSPEEDCODE,
    SYS_IF_TDMLIULOSAIS,
    SYS_IF_TDMLIUOUTPUT,
    SYS_IF_TDMVCSTATE,
    SYS_IF_TDMLOCALMAC,
    SYS_IF_TDMLOCALIPADDR,
    SYS_IF_TDMREMOTEMAC,
    SYS_IF_TDMREMOTEIPADDR,
    SYS_IF_TDMREMOTEIF,
    SYS_IF_TDMENCAPTYPE,
    SYS_IF_TDMMAXVC,
    SYS_IF_TDMMPLSINNERLABEL,
    SYS_IF_TDMMPLSINNERLABELEXP,
    SYS_IF_TDMMPLSINNERLABELSTACK,
    SYS_IF_TDMMPLSINNERLABELTTL,
    SYS_IF_TDMMPLSOUTERLABEL1,
    SYS_IF_TDMMPLSOUTERLABEL1EXP,
    SYS_IF_TDMMPLSOUTERLABEL1STACK,
    SYS_IF_TDMMPLSOUTERLABEL1TTL,
    SYS_IF_TDMMPLSOUTERLABEL2,
    SYS_IF_TDMMPLSOUTERLABEL2EXP,
    SYS_IF_TDMMPLSOUTERLABEL2STACK,
    SYS_IF_TDMMPLSOUTERLABEL2TTL,
    SYS_IF_TDME1FRAMEMODE,
    SYS_IF_TDME1CLOCKMODE,
    SYS_IF_TDME1ENCAPSULATETYPE,
    SYS_IF_TDME1PAYLOADFRAME,
    SYS_IF_TDME1ENABLE,
    SYS_IF_TDME1LOOPENABLE,
    SYS_IF_TDMRTPHEADENABLE,
    SYS_IF_TDMRTPTIMESTAMPMODE,
    SYS_IF_TDME1JITTERBUFFER,
    SYS_IF_TDME1TIMESLOT,
    SYS_IF_TDME1PAYLOADTXCOUNTER,
    SYS_IF_TDME1PAYLOADRXCOUNTER,
    SYS_IF_TDME1PAYLOADDROPCOUNTER,
    SYS_IF_TDME1CRCCOUNTER,
    SYS_IF_TDME1ALARM,



    /*tunnel cmd*/
    SYS_IF_TUNNELID=5200,
    SYS_IF_TUNNELENCAPSMETHOD,
    SYS_IF_TUNNELCHECKSUM,
    SYS_IF_TUNNELSECURITY,
    SYS_IF_TUNNELIPADDRTYPE,
    SYS_IF_TUNNELSRCIP,
    SYS_IF_TUNNELDSTIP,
    SYS_IF_TUNNELHOPLIMIT,
    SYS_IF_TUNNELFLOWLABEL,
    SYS_IF_TUNNELENCAPSLIMIT,
    SYS_IF_TUNNELDSCPMETHOD,
    SYS_IF_TUNNELALLKEY,
    SYS_IF_TUNNEL6RDIPV6PREFIX,
    SYS_IF_TUNNEL6RDIPV6PREFIXLENGTH,
    SYS_IF_TUNNEL6RDBRIPV4ADDR,
    SYS_IF_TUNNEL6RDIPV4PREFIXLENGTH,
    SYS_IF_TUNNELSRCIFINDEX,                /*uint32_t ,if unit,*/
    SYS_IF_TUNNELCFGTUNNELID,
    SYS_IF_TUNNELDSTVRFID,

    /*vlan cmd*/
    SYS_IF_VLANVID=5400,
    SYS_IF_VLANVLANSTATUS,
    SYS_IF_VLANVLANTYPE,
    SYS_IF_VLANVLANMODE,
    SYS_IF_VLANADDSECONDARYVLAN,
    SYS_IF_VLANDELSECONDARYVLAN,
    SYS_IF_VLANDELALLSECONDARYVLAN, /*ɾ��ָ��vlan��ȫ����vlan*/
    SYS_IF_VLANMEMBERPORTNUM,   /*��ȡvlan�ӿ��ڽӿڳ�Ա����*/
    SYS_IF_VLANSTATICMEMBERPORTNUM, /*��ȡvlan�ӿ��ھ�̬�ӿڳ�Ա����*/
    SYS_IF_VLANDYNAMICMEMBERPORTNUM,    /*��ȡvlan�ӿ��ڶ�̬�ӿڳ�Ա����*/
    SYS_IF_VLANEGRESSPORTS,             /*��ȡvlan�ӿ��ڽӿڳ�Ա��������̬����̬��Ա,var��������Ϊoctetstringָ�룬ȫ�����˿ڡ�
                                        ÿ�ĸ��ֽڱ�ʾһ�˿ڣ��������ֽ���洢�˿ڵ�unit��*/
    SYS_IF_VLANEGRESSPORTBITS,      /*��ȡvlan�ӿ��ڽӿڳ�Աbitλ*/
    SYS_IF_VLANSTATICEGRESSPORTS,
    SYS_IF_VLANSTATICEGRESSPORTBITS,
    SYS_IF_VLANDYNAMICEGRESSPORTS,
    SYS_IF_VLANDYNAMICEGRESSPORTBITS,
    SYS_IF_VLANTAGGEDEGRESSPORTS,
    SYS_IF_VLANTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANUNTAGGEDEGRESSPORTS,
    SYS_IF_VLANUNTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANSTATICTAGGEDEGRESSPORTS,
    SYS_IF_VLANSTATICTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANSTATICUNTAGGEDEGRESSPORTS,
    SYS_IF_VLANSTATICUNTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANDYNAMICTAGGEDEGRESSPORTS,
    SYS_IF_VLANDYNAMICTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANDYNAMICUNTAGGEDEGRESSPORTS,
    SYS_IF_VLANDYNAMICUNTAGGEDEGRESSPORTBITS,
    SYS_IF_VLANSECONDARYVLAN,
    SYS_IF_VLANPRIMARYVLAN,
    SYS_IF_VLANINNERVLANARPPROXY,
    SYS_IF_VLANINTERVLANARPPROXY,
    SYS_IF_VLANINNERVLANNDPROXY,
    SYS_IF_VLANINTERVLANNDPROXY,
    SYS_IF_VLANSTATSENABLE,         /*vlanӲ��ͳ�ƹ����Ƿ���ܣ�ȡֵ�μ�USP_IF_ENABLE*/
    SYS_IF_VLANCMDSTART=5600,


    /*pon if*/
    SYS_IF_PONFECENABLE=5700,
    SYS_IF_PONRANGINGENABLE,
    SYS_IF_PONONUAUTHENABLE,
    SYS_IF_PONMAXRTT,
    SYS_IF_PONGRANTFILTERENABLE,
    SYS_IF_PONGETONUID,     /*����Ϊoctetstring*/
    SYS_IF_PONADDONUID,     /*����Ϊuint32_t*/
    SYS_IF_PONDELONUID,     /*����Ϊuint32_t*/
    SYS_IF_PONGETFREEONUID, /*get,��ȡ����onuid,uint32_t*/
    SYS_IF_PONTSTONUID,     /*get,̽��onuid�Ƿ����,uint32_t*/
    SYS_IF_PONMAXONUNUM,    /*set/get�ӿ����onu����,uint32_t*/
    SYS_IF_PONONUISOLATE,   /*get&set,uint32_t,pon����onu����ʹ��*/
    SYS_IF_PONENCRYPTMODE,  /*get&set,uint32_t,����ģʽ*/
    SYS_IF_PONOPTICALADJ,   /*get&set,uint32_t,pon�ڹ⹦�ʵ���*/
    SYS_IF_PONENCRYPTUPDATETIME,    /*get&set,uint32_t,�������ʱ��*/
    SYS_IF_PONPSTYPE,           /*pon�ڱ�����������,int32_t,ȡֵΪUSP_PON_PSTYPEA��*/
    SYS_IF_PONFIR,              /*ָ���ӿ���������Ȩonu�̴���֮��,uint32_t*,����dbaģ����ò���Ҫ��ͬ��*/
    SYS_IF_PONFIRTHRESHOLD,     /*ָ���ӿ���������Ȩonu�̴���֮������,uint32_t**/
    SYS_IF_PONMCASTMINBW,       /*�鲥��С����,unit:kbps,default:0(�ر�),uint32_t *,*/
    SYS_IF_PONMCASTMAXBW,       /*�鲥������,unit:kbps,default:0(�ر�),uint32_t *,*/



    /*loopback if*/
    SYS_IF_LOOPBACKID=5800,


    /*null if*/
    SYS_IF_NULLID=5900,

    /*ds1 if, rfc4805*/
    SYS_IF_DS1CHANNELNUMBER=6000,
    SYS_IF_DS1CHANNELIZTION,
    SYS_IF_DS1LINETYPE,
    SYS_IF_DS1LINECODE,
    SYS_IF_DS1SENDCODE,
    SYS_IF_DS1LOOPBACKCFG,
    SYS_IF_DS1SIGNALMODE,
    SYS_IF_DS1CLOCKSOURCE,
    SYS_IF_DS1STATUSCHANGETRAPENABLE,
    SYS_IF_DS1LINEIMPEDANCE,
    SYS_IF_DS1CRC,

    SYS_IF_DS1LINESTATUS,
    SYS_IF_DS1LOOPBACKSTATUS,
    SYS_IF_DS1LINESTATUSLASTCHANGE,

    SYS_IF_DS1USEDTIMESLOTS,
    SYS_IF_DS1USEDCHANNELS,
    SYS_IF_DS1ADDCHANNEL,   /*only set, octetstring,1byte channel number,4btyes speed, 4bytes timeslots*/
    SYS_IF_DS1DELCHANNEL,   /*only set, octetstring,1byte channel number*/
    SYS_IF_DS1REMOVEDELCHANNEL,


    SYS_IF_DS1CHANNELTIMESLOTS,/*only set, octetstring,1byte channel number,4bytes timeslots*/
    SYS_IF_DS1ALLCHANNELS,  /*only get, octetstring*/

    SYS_IF_DSX1CURRENTESS,
    SYS_IF_DSX1CURRENTSESS,
    SYS_IF_DSX1CURRENTSEFSS,
    SYS_IF_DSX1CURRENTUASS,
    SYS_IF_DSX1CURRENTCSSS,
    SYS_IF_DSX1CURRENTPCVS,
    SYS_IF_DSX1CURRENTLESS,
    SYS_IF_DSX1CURRENTBESS,
    SYS_IF_DSX1CURRENTDMS,
    SYS_IF_DSX1CURRENTLCVS,

    SYS_IF_DSX1TOTALESS,
    SYS_IF_DSX1TOTALSESS,
    SYS_IF_DSX1TOTALSEFSS,
    SYS_IF_DSX1TOTALUASS,
    SYS_IF_DSX1TOTALCSSS,
    SYS_IF_DSX1TOTALPCVS,
    SYS_IF_DSX1TOTALLESS,
    SYS_IF_DSX1TOTALBESS,
    SYS_IF_DSX1TOTALDMS,
    SYS_IF_DSX1TOTALLCVS,


    /*ds3 if, rfc3896*/
    SYS_IF_DS3CHANNELNUMBER=6200,
    SYS_IF_DS3CHANNELIZTION,
    SYS_IF_DS3LINETYPE,
    SYS_IF_DS3LINECODE,
    SYS_IF_DS3SENDCODE,
    SYS_IF_DS3LOOPBACKCFG,
    SYS_IF_DS3SIGNALMODE,
    SYS_IF_DS3CLOCKSOURCE,
    SYS_IF_DS3STATUSCHANGETRAPENABLE,
    SYS_IF_DS3LINEIMPEDANCE,
    SYS_IF_DS3CRC,

    SYS_IF_DS3LINESTATUS,
    SYS_IF_DS3LOOPBACKSTATUS,
    SYS_IF_DS3LINESTATUSLASTCHANGE,

    SYS_IF_DS3USEDCHANNELS,     /*octetstring, bitListSet/Tst*/
    SYS_IF_DS3ADDCHANNEL,   /*
                                ����Ϊoctetstring,
                                �����ṹ����:
                                1 bytes          1 bytes      1 btyes
                                channelnumber    linetype     channelization
                                */
    SYS_IF_DS3DELCHANNEL,   /*
                                ����Ϊoctetstring,
                                �����ṹ����:
                                1 bytes          1 bytes      1 btyes
                                channelnumber    linetype     channelization
                                */
    SYS_IF_DS3REMOVEDELCHANNEL,

    SYS_IF_DS3ALLCHANNELS,  /*octetstring,1byte channel, 1byte linetype, 1byte channelization, 4byte ifindex*/

    SYS_IF_DSX3CURRENTPESS,
    SYS_IF_DSX3CURRENTPSESS,
    SYS_IF_DSX3CURRENTSEFSS,
    SYS_IF_DSX3CURRENTUASS,
    SYS_IF_DSX3CURRENTLCVS,
    SYS_IF_DSX3CURRENTPCVS,
    SYS_IF_DSX3CURRENTLESS,
    SYS_IF_DSX3CURRENTCCVS,
    SYS_IF_DSX3CURRENTCESS,
    SYS_IF_DSX3CURRENTCSESS,

    SYS_IF_DSX3TOTALPESS,
    SYS_IF_DSX3TOTALPSESS,
    SYS_IF_DSX3TOTALSEFSS,
    SYS_IF_DSX3TOTALUASS,
    SYS_IF_DSX3TOTALLCVS,
    SYS_IF_DSX3TOTALPCVS,
    SYS_IF_DSX3TOTALLESS,
    SYS_IF_DSX3TOTALCCVS,
    SYS_IF_DSX3TOTALCESS,
    SYS_IF_DSX3TOTALCSESS,

    /*serial if*/
    SYS_IF_SERIALCRC=7000,

    /*docsis if*/
    SYS_IF_DOCSISADDCHANNEL=7300,
    SYS_IF_DOCSISDELCHANNEL,

    /*docsis if upstream channel*/
    SYS_IF_DOCSISUPCHANNELID=7600,
    SYS_IF_DOCSISUPCHANNELFREQUENCY,                        /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELWIDTH,                            /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELMODULATIONPROFILE,                /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSLOTSIZE,                     /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELTXTIMINGOFFSET,                   /*int, only get*/
    SYS_IF_DOCSISUPCHANNELRANGINGBACKOFFSTART,          /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELRANGINGBACKOFFEND,            /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELTXBACKOFFSTART,                   /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELTXBACKOFFEND,                 /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSCDMAACTIVECODES,             /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSCDMACODESPERSLOT,                /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSCDMAFRAMESIZE,                   /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSCDMAHOPPINGSEED,             /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELTYPE,                         /*int, only get*/
    SYS_IF_DOCSISUPCHANNELCLONEFROM,                        /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELUPDATE,                           /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELSTATUS,                       /*int, get&set,rowstatus*/
    SYS_IF_DOCSISUPCHANNELPREEQENABLE,                  /*int, get&set*/
    SYS_IF_DOCSISUPCHANNELEXPECTEDRECEIVEDSIGNALPOWER,  /*int, get&set,TenthdBmV*/


    /*docsis if dowNSTREAM CHANNEL*/
    SYS_IF_DOCSISDOWNCHANNELID=7900,
    SYS_IF_DOCSISDOWNCHANNELFREQUENCY,                  /*int, get&set*/
    SYS_IF_DOCSISDOWNCHANNELWIDTH,                      /*int, get&set*/
    SYS_IF_DOCSISDOWNCHANNELMODULATION,                 /*int, get&set*/
    SYS_IF_DOCSISDOWNCHANNELINTERLEAVE,                 /*int, get&set*/
    SYS_IF_DOCSISDOWNCHANNELPOWER,                      /*int, get&set*/
    SYS_IF_DOCSISDOWNCHANNELANNEX,                      /*int, only get*/
    SYS_IF_DOCSISDOWNCHANNELUPDATE,                     /*int, get&set*/

    SYS_IF_V35LOCALALARM=8200,
    SYS_IF_V35LOCALCLOCK,
    SYS_IF_V35EDGEAUTO,
    SYS_IF_V35LOCALTXDATA,
    SYS_IF_V35LOCALRXDATA,
    SYS_IF_V35REMOTEALARM,
    SYS_IF_V35REMOTECLOCK,
    SYS_IF_V35REMOTESTATUS,
    SYS_IF_V35REMOTETXDATA,
    SYS_IF_V35REMOTERXDATA,
    SYS_IF_V35PATTTESTENABLE,
    SYS_IF_V35PATTTESTSTATUS,
    SYS_IF_V35PATTTESTRESULT,
    SYS_IF_V35LFPENABLE,
    SYS_IF_V35TXEDGECTRL,
    SYS_IF_V35RXEDGECTRL,
    SYS_IF_V35ANALOOP,
    SYS_IF_V35DIGLOOP,
    SYS_IF_V35REMOTELOOP,
    SYS_IF_V35NEIGHBOREXIST,
    SYS_IF_V35LOOPDETECTENABLE,
    SYS_IF_V35LOOPDETECTSTATUS,


}SYS_IF_CMD;




typedef enum{
    SYS_PHYSICAL_NUM,
    SYS_LOGICAL_NUM,
    SYS_PHY2INDEX_NUM,
    SYS_INDEX2PHY_NUM,
    SYS_LOGICAL2INDEX_NUM,
    SYS_INDEX2LOGICAL_NUM,
    SYS_LOGICAL2PHY_NUM,
    SYS_PHY2LOGICAL_NUM,
    SYS_IPINDEX2LOGICAL_NUM,
    SYS_LOGICAL2IPINDEX_NUM,
    SYS_IPADDR2LOGICAL_NUM, /*����ip��ַ����unit��*/
    SYS_IPNET2LOGICAL_NUM,  /*����ip���β���unit��,�ƥ��*/
    SYS_IPNETMASK2LOGICAL_NUM,  /*����ip���Ρ��������unit��*/
    SYS_IPADDRISLOGICAL_NUM,    /*����ip��ַ�Ƿ�Ϊ���ص�ַ*/
    SYS_IPGET_MATCH_IF_BY_IP,
    SYS_FANLOGICAL2PHY,
    SYS_FANPHY2LOGICAL,

    SYS_POWERLOGICAL2PHY,
    SYS_POWERPHY2LOGICAL,

    SYS_CPULOGICAL2PHY,
    SYS_CPUPHY2LOGICAL,

    SYS_MEMORYLOGICAL2PHY,
    SYS_MEMORYPHY2LOGICAL,

    SYS_VOLTAGELOGICAL2PHY,
    SYS_VOLTAGEPHY2LOGICAL,

    SYS_TEMPERATURELOGICAL2PHY,
    SYS_TEMPERATUREPHY2LOGICAL,

    SYS_ENTITY_LOGICAL2PHY,
    SYS_ENTITY_PHY2LOGICAL,

    SYS_VPN_NAME2VRF,
    SYS_VPN_VRF2NAME,

    SYS_IPNET2IFADDR,   /*����ip��ַ�����β��ҽӿ�ip,����,�����Ϊoctetstring*/
    SYS_DSTIP2OUTIFADDR,    /*����Ŀ��ip����·����һ����Ӧ�ĳ��ӿ�ip��ַ*/
    SYS_DSTIP2OUTIFINDEX,   /*����Ŀ��ip����·����һ����Ӧ�ĳ��ӿ�ifindex*/
    SYS_DSTIP2NEXTHOPIPADDR,    /*����Ŀ��ip����·����һ����ַ*/
    SYS_DSTIP2NEXTHOPOUTIFINDEX,    /*����Ŀ��ip����·����һ�����ӿ�*/
    SYS_DSTIP2NEXTHOPINFO,  /*����Ŀ��ip����·����һ����Ӧ�ĳ��ӿ�ifindex,��һ��IP��ַ����һ��mac��ַ,ifindex+macΪ0��ʾδѧ����һ����arp����*/

    SYS_L2VPN_NAME2VFI,
    SYS_L2VPN_VFI2NAME,

    SYS_IPGREATERNETMASK2LOGICAL,   /*���Ҵ��ڵ���ָ�������ַ�ͳ��ȵ���ı��ؽӿ�*/
    SYS_IPGREATERNETMASK2ADDR,      /*���Ҵ��ڵ���ָ�������ַ�ͳ��ȵ���ı��ص�ַ*/
    SYS_PROCESS_IPNET_MASK_BING_VRF,
    SYS_PROTO_ENABLE,

}SYS_INDEXTRANS_CMD;

#define SNMP_ACTIVE          1
#define SNMP_NOTINSERVICE    2
#define SNMP_NOTREADY        3
#define SNMP_CREATEANDGO     4
#define SNMP_CREATEANDWAIT   5
#define SNMP_DESTROY         6

#define IP_UNKNOWN     0
#define IP_V4FAMILY    1
#define IP_V6FAMILY    2

#define USP_SYNC_DOWNSTREAM        0x1
#define USP_SYNC_HORIZONTAL        0x2
#define USP_SYNC_UPSTREAM          0x4
#define USP_SYNC_NONBLOCK          0x10
#define USP_SYNC_OCTETDATA         0x80000000

#define USP_SYNC_LOCAL    USP_SYNC_DOWNSTREAM
#define USP_SYNC_REMOTE   USP_SYNC_HORIZONTAL

BOOL ospf_if_is_trunk(ifindex_t n);
BOOL ospf_if_is_loopback(ifindex_t n);

u_int	ospf_if_get_mtu(ifindex_t ifindex);

#endif /* __OSPF_PAL_H__ */
