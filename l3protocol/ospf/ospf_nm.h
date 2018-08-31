/* ospf_nm.h*/

#ifndef __OSPF_NM_H
#define __OSPF_NM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#include "ldp_nm.h"
#include "ospf_relation.h"

#define OSPF_ASSERT(condition,retValue)\
do{\
    if(!(condition))\
    {\
        vty_out_to_all_terminal("\r\n ospf assert error in line %d in func %s.",__LINE__,__FUNCTION__);\
        return retValue;\
    }\
}while(0)

#define ospf_status(x) ((x)?TRUE:FALSE)

#define OSPF_CMD_MAXDATALEN 1500
#define OSPF_PATH "/tmp/ospf.socket"

#define SYS_IF_ADMIN_STATUS		SYS_IF_ADMINSTATUS
#define SYS_IF_OPER_STATUS		SYS_IF_OPERSTATUS

#define SYS_IF_QOS_ALG				SYS_IF_QUEUEPOLICY
#define SYS_IF_QOS_SP_BMP			SYS_IF_QUEUESPBMP
#define SYS_IF_QOS_MAXQUEUE			SYS_IF_QUEUEMAXNUM
#define SYS_IF_COS_MODIFY_PORTLIST	SYS_IF_QUEUEPOLICYMODIFYPORTLIST
#define SYS_IF_BW_SUPPORT			SYS_IF_QUEUEBWSUPPORT
#define SYS_IF_QOS_ALG_SUPPORT		SYS_IF_QUEUEALGSUPPORT
#define SYS_IF_COS_MIN_BW_SUPPORT	SYS_IF_QUEUEMINBWSUPPORT
#define SYS_IF_COS_MAX_BW_SUPPORT	SYS_IF_QUEUEMAXBWSUPPORT
#define SYS_IF_QOS_WEIGHT_SUPPORT	SYS_IF_QUEUEWEIGHTSUPPORT
	
#define SYS_IF_FLUSH_MAC	SYS_IF_FLUSHMAC
#define SYS_IF_FLUSH_ARP SYS_IF_FLUSHARP
#define OSPF_NHOP_MAXCOUNT 64

#define OSPF_LOG_INF(format, ...) zlog(ZLOG_OSPF, LOG_INFO, format"(%s:%d)", ## __VA_ARGS__, __FILE__, __LINE__)
#define OSPF_LOG_DBG(format, ...) zlog(ZLOG_OSPF, LOG_DEBUG, format"(%s:%d)", ## __VA_ARGS__,__FILE__, __LINE__)
#define OSPF_LOG_WARN(format, ...) zlog(ZLOG_OSPF, LOG_WARNING, format"(%s:%d)", ## __VA_ARGS__,__FILE__, __LINE__)
#define OSPF_LOG_ERR(format,...) zlog(ZLOG_OSPF, LOG_ERR, format"(%s:%d)", ## __VA_ARGS__, __FILE__, __LINE__)

#define OSPF_PROCESS_COUNT 100
#define OSPF_DCN_LOOPBACK_INDEX 63

enum {
    OSPF_GLOBAL_SETAPI = 1,
    OSPF_GLOBAL_GETAPI,
    OSPF_GLOBAL_SYNCAPI,
    
    OSPF_INSTANCE_GETFIRST,
    OSPF_INSTANCE_GETNEXT,
    OSPF_INSTANCE_SETAPI,
    OSPF_INSTANCE_GETAPI,
    OSPF_INSTANCE_SYNCAPI,
        
    OSPF_AREA_GETFIRST,
    OSPF_AREA_GETNEXT,
    OSPF_AREA_SETAPI,
    OSPF_AREA_GETAPI,
    OSPF_AREA_SYNCAPI,

    OSPF_STUBAREA_GETFIRST,
    OSPF_STUBAREA_GETNEXT,
    OSPF_STUBAREA_SETAPI,
    OSPF_STUBAREA_GETAPI,
    OSPF_STUBAREA_SYNCAPI,

    OSPF_LSBD_GETFIRST,
    OSPF_LSBD_GETNEXT,
    OSPF_LSBD_SETAPI,
    OSPF_LSBD_GETAPI,
    OSPF_LSBD_SYNCAPI,
    
    OSPF_EXTLSBD_GETFIRST,
    OSPF_EXTLSBD_GETNEXT,
    OSPF_EXTLSBD_SETAPI,
    OSPF_EXTLSBD_GETAPI,
    OSPF_EXTLSBD_SYNCAPI,

    OSPF_ASLSBD_GETFIRST,
    OSPF_ASLSBD_GETNEXT,
    OSPF_ASLSBD_SETAPI,
    OSPF_ASLSBD_GETAPI,
    OSPF_ASLSBD_SYNCAPI,

    OSPF_IFLSBD_GETFIRST,
    OSPF_IFLSBD_GETNEXT,
    OSPF_IFLSBD_SETAPI,
    OSPF_IFLSBD_GETAPI,
    OSPF_IFLSBD_SYNCAPI,

    OSPF_VIFLSBD_GETFIRST,
    OSPF_VIFLSBD_GETNEXT,
    OSPF_VIFLSBD_SETAPI,
    OSPF_VIFLSBD_GETAPI,
    OSPF_VIFLSBD_SYNCAPI,
    
    OSPF_AREALSACOUNT_GETFIRST,
    OSPF_AREALSACOUNT_GETNEXT,
    OSPF_AREALSACOUNT_SETAPI,
    OSPF_AREALSACOUNT_GETAPI,
    OSPF_AREALSACOUNT_SYNCAPI,

    OSPF_AREARANGE_GETFIRST,
    OSPF_AREARANGE_GETNEXT,
    OSPF_AREARANGE_SETAPI,
    OSPF_AREARANGE_GETAPI,
    OSPF_AREARANGE_SYNCAPI,

    OSPF_AREAAGGREGATE_GETFIRST,
    OSPF_AREAAGGREGATE_GETNEXT,
    OSPF_AREAAGGREGATE_SETAPI,
    OSPF_AREAAGGREGATE_GETAPI,
    OSPF_AREAAGGREGATE_SYNCAPI,

    OSPF_NSSARANGE_GETFIRST,
    OSPF_NSSARANGE_GETNEXT,
    OSPF_NSSARANGE_SETAPI,
    OSPF_NSSARANGE_GETAPI,
    OSPF_NSSARANGE_SYNCAPI,

    OSPF_IF_GETFIRST,
    OSPF_IF_GETNEXT,
    OSPF_IF_SETAPI,
    OSPF_IF_GETAPI,
    OSPF_IF_SYNCAPI,

    OSPF_IFMETRIC_GETFIRST,
    OSPF_IFMETRIC_GETNEXT,
    OSPF_IFMETRIC_SETAPI,
    OSPF_IFMETRIC_GETAPI,
    OSPF_IFMETRIC_SYNCAPI,

    OSPF_VIF_GETFIRST,
    OSPF_VIF_GETNEXT,
    OSPF_VIF_SETAPI,
    OSPF_VIF_GETAPI,
    OSPF_VIF_SYNCAPI,

    OSPF_NBR_GETFIRST,
    OSPF_NBR_GETNEXT,
    OSPF_NBR_SETAPI,
    OSPF_NBR_GETAPI,
    OSPF_NBR_SYNCAPI,
    
    OSPF_VNBR_GETFIRST,
    OSPF_VNBR_GETNEXT,
    OSPF_VNBR_SETAPI,
    OSPF_VNBR_GETAPI,
    OSPF_VNBR_SYNCAPI,

    OSPF_NETWORK_GETFIRST,
    OSPF_NETWORK_GETNEXT,
    OSPF_NETWORK_SETAPI,
    OSPF_NETWORK_GETAPI,
    OSPF_NETWORK_SYNCAPI,

    OSPF_REDRANGE_GETFIRST,
    OSPF_REDRANGE_GETNEXT,
    OSPF_REDRANGE_SETAPI,
    OSPF_REDRANGE_GETAPI,
    OSPF_REDRANGE_SYNCAPI,

    OSPF_DISTRIBUTE_GETFIRST,
    OSPF_DISTRIBUTE_GETNEXT,
    OSPF_DISTRIBUTE_SETAPI,
    OSPF_DISTRIBUTE_GETAPI,
    OSPF_DISTRIBUTE_SYNCAPI,
    
    OSPF_NETWORKROUTE_GETFIRST,
    OSPF_NETWORKROUTE_GETNEXT,
    OSPF_NETWORKROUTE_SETAPI,
    OSPF_NETWORKROUTE_GETAPI,
    OSPF_NETWORKROUTE_SYNCAPI,

    OSPF_ABRROUTE_GETFIRST,
    OSPF_ABRROUTE_GETNEXT,
    OSPF_ABRROUTE_SETAPI,
    OSPF_ABRROUTE_GETAPI,
    OSPF_ABRROUTE_SYNCAPI,
    
    OSPF_ASBRROUTE_GETFIRST,
    OSPF_ASBRROUTE_GETNEXT,
    OSPF_ASBRROUTE_SETAPI,
    OSPF_ASBRROUTE_GETAPI,
    OSPF_ASBRROUTE_SYNCAPI,

    OSPF_REDPOLICY_GETFIRST,
    OSPF_REDPOLICY_GETNEXT,
    OSPF_REDPOLICY_SETAPI,
    OSPF_REDPOLICY_GETAPI,
    OSPF_REDPOLICY_SYNCAPI,
    
    OSPF_FILTERPOLICY_GETFIRST,
    OSPF_FILTERPOLICY_GETNEXT,
    OSPF_FILTERPOLICY_SETAPI,
    OSPF_FILTERPOLICY_GETAPI,
    OSPF_FILTERPOLICY_SYNCAPI,
    
    OSPF_SHAMLINK_GETFIRST,
    OSPF_SHAMLINK_GETNEXT,
    OSPF_SHAMLINK_SETAPI,
    OSPF_SHAMLINK_GETAPI,
    OSPF_SHAMLINK_SYNCAPI,

    OSPF_UTIL_SETAPI,
    OSPF_UTIL_GETAPI
};




/*OSPF Version 2 Management Information Base RFC 4750*/

/*global object*/

/*OspfAuthenticationType {
                       none (0),
                       simplePassword (1),
                       md5 (2)
                      reserved for specification by IANA (> 2)
                    }
*/
enum {
           OSPF_AUTH_NONE = 0,
           OSPF_AUTH_SIMPLE,
           OSPF_AUTH_MD5 
};

enum {
           OSPF_AUTHDIS_NONE = 0,
           OSPF_AUTHDIS_CIPHER,
           OSPF_AUTHDIS_PLAIN
};

enum
{
    OSPF_RESTART_NONE = 1, 
    OSPF_RESTART_IN_PROGRESS,
    OSPF_RESTART_COMPLETED,
    OSPF_RESTART_TIMEOUT,
    OSPF_RESTART_TOPLOGY_CHANGED,
} ;

enum {
    OSPF_GRSUPPORT_NONE = 1,
    OSPF_GRSUPPORT_PLANNEDONLY,
    OSPF_GRSUPPORT_PLANNEDANDUNPLANNED
};

enum
{
    OSPF_RESTART_NO_RESTARTING = 1, 
    OSPF_RESTART_PLANNED,
    OSPF_RESTART_UNPLANNED,
} ; 

enum
{
    OSPF_RESTART_NOT_HELPING = 1, 
    OSPF_RESTART_HELPING,
} ; 

enum{
    OSPF_TRAP_BIT_VIFSTATE = 1,
    OSPF_TRAP_BIT_NBRSTATE, 
    OSPF_TRAP_BIT_VNBRSTATE,
    OSPF_TRAP_BIT_IFERROR,
    OSPF_TRAP_BIT_VIFERROR,
    OSPF_TRAP_BIT_IFAUTHERROR,
    OSPF_TRAP_BIT_VIFAUTHERROR,
    OSPF_TRAP_BIT_IFBADPKT,
    OSPF_TRAP_BIT_VIFBADPKT,
    OSPF_TRAP_BIT_IFRXMT,
    OSPF_TRAP_BIT_VIFRXMT,
    OSPF_TRAP_BIT_ORIGIN,
    OSPF_TRAP_BIT_MAXAGE,
    OSPF_TRAP_BIT_OVERFLOW,
    OSPF_TRAP_BIT_NEAROVERFLOW,
    OSPF_TRAP_BIT_IFSTATE,
    OSPF_TRAP_BIT_TRANSLATOR,
    OSPF_TRAP_BIT_RESTARTSTUTAS,
    OSPF_TRAP_BIT_HELPERSTUTAS,
    OSPF_TRAP_BIT_VIRHELPERSTUTAS

};
/*debug flag*/
 enum {
    OSPF_DBG_MAIN = 0,
    OSPF_DBG_LSA,
    OSPF_DBG_HELLO,
    OSPF_DBG_PACKET,
    OSPF_DBG_IF,
    OSPF_DBG_NBR,
    OSPF_DBG_ROUTE,
    OSPF_DBG_SPF,
    OSPF_DBG_RTM,
    OSPF_DBG_SYN,
    OSPF_DBG_GR,
    OSPF_DBG_NBRCHANGE,
    OSPF_DBG_FRR,
    OSPF_DBG_ERROR,
    OSPF_DBG_DCN,
    OSPF_DBG_POLICY,
    OSPF_DBG_BFD,
    OSPF_DBG_SYSLOG,
    OSPF_DBG_MAX,
 };
/*  
  ospfRouterId 
  ospfAdminStat Status
  ospfVersionNumber 
  ospfAreaBdrRtrStatus  TruthValue
  ospfASBdrRtrStatus  TruthValue
  ospfExternLsaCount 
  ospfExternLsaCksumSum 
  ospfTOSSupport TruthValue
  ospfOriginateNewLsas 
  ospfRxNewLsas 
  ospfExtLsdbLimit 
  ospfMulticastExtensions TruthValue
  ospfDemandExtensions  TruthValue
  ospfRFC1583Compatibility  TruthValue
  ospfOpaqueLsaSupport  TruthValue
  ospfReferenceBandwidth 
  ospfRestartSupport { none (1), plannedOnly (2), plannedAndUnplanned (3)}
  ospfRestartInterval 
  ospfRestartStrictLsaChecking TruthValue
  ospfRestartStatus { notRestarting (1), plannedRestart (2), unplannedRestart (3)}
  ospfRestartAge 
  ospfRestartExitReason { none (1), inProgress (2), completed (3), timedOut (4), topologyChanged (5)}
  ospfAsLsaCount 
  ospfAsLsaCksumSum 
  ospfStubRouterSupport  TruthValue
  ospfStubRouterAdvertisement  {doNotAdvertise (1), advertise(2)}
  ospfDiscontinuityTime 
   */
enum {
    OSPF_GBL_ROUTERID   ,/*ospfRouterId*/
    OSPF_GBL_ADMIN,/*ospfAdminStat*/
    OSPF_GBL_VER,/*ospfVersionNumber*/
    OSPF_GBL_ABRSTATE,/*ospfAreaBdrRtrStatus*/
    OSPF_GBL_ASBRSTATE,/*ospfASBdrRtrStatus*/
    OSPF_GBL_EXTLSACOUNT,/*ospfExternLsaCount*/
    OSPF_GBL_EXTLSACHECKSUM,/*ospfExternLsaCksumSum*/
    OSPF_GBL_TOSSUPPORT,/*ospfTOSSupport*/
    OSPF_GBL_ORIGINNEWLSA,/*ospfOriginateNewLsas*/
    OSPF_GBL_RXDNEWLSA,/*ospfRxNewLsas*/
    OSPF_GBL_EXTLSDBLIMIT,/*ospfExtLsdbLimit*/
    OSPF_GBL_MCASTEXTERN,/*ospfMulticastExtensions*/
    OSPF_GBL_OVERFLOWINTERVAL,
    OSPF_GBL_DEMANDEXTERN ,  /*ospfDemandExtensions*/
    OSPF_GBL_RFC1583COMPATIBILITY,/*ospfRFC1583Compatibility*/
    OSPF_GBL_OPAQUE,/*ospfOpaqueLsaSupport*/
    OSPF_GBL_REFRATE,   /*ospfReferenceBandwidth*/
    OSPF_GBL_RESTARTSUPPORT, /*ospfRestartSupport*/ 
    OSPF_GBL_RESTARTINTERVAL,/*ospfRestartInterval */
    OSPF_GBL_RESTARTSTRICLSACHECKING,/*ospfRestartStrictLsaChecking */
    OSPF_GBL_RESTARTSTATUS,/*ospfRestartStatus */
    OSPF_GBL_RESTARTAGE,/*ospfRestartAge */
    OSPF_GBL_RESTARTEXITREASON,/*ospfRestartExitReason*/
    OSPF_GBL_ASLSACOUNT,/*ospfAsLsaCount */
    OSPF_GBL_ASLSACHECKSUM,/*ospfAsLsaCksumSum */
    OSPF_GBL_STUBROUTERSUPPORT,/*ospfStubRouterSupport */
    OSPF_GBL_STUBROUTERADVERTISEMENT,/*ospfStubRouterAdvertisement  {doNotAdvertise (1), advertise(2)}*/
    OSPF_GBL_STUBROUTERADVERTISEMENT_TIME,
    OSPF_GBL_DISCONTINUITYTIME ,/*ospfDiscontinuityTime  */
    OSPF_GBL_REDISTRIBUTELOCAL , 
    OSPF_GBL_REDISTRIBUTERIP ,
    OSPF_GBL_REDISTRIBUTEBGP,   
    OSPF_GBL_REDISTRIBUTESTATIC,  
    OSPF_GBL_REDISTRIBUTEISIS,
    OSPF_GBL_REDISTRIBUTEOSPF,
    OSPF_GBL_TRAP,
    OSPF_GBL_TRAPERROR,
    OSPF_GBL_TRAPTYPE,
    OSPF_GBL_TRAPSRC,
    OSPF_GBL_SPFINTERVAL ,
    OSPF_GBL_RESTARTHELPLERENABLE,
    OSPF_GBL_RESTARTREASON,
    OSPF_GBL_DEBUGGLOBAL , 
    OSPF_GBL_DEBUGPACKET, 
    OSPF_GBL_DEBUGHELLO , 
    OSPF_GBL_DEBUGIF, 
    OSPF_GBL_DEBUGNBR, 
    OSPF_GBL_DEBUGLSA, 
    OSPF_GBL_DEBUGIPROUTE, 
    OSPF_GBL_DEBUGROUTECHANGE, 
    OSPF_GBL_DEBUGSPF, 
    OSPF_GBL_DEBUGSYN,
    OSPF_GBL_DEBUGGR,
    OSPF_GBL_DEBUGNBRCHANGE,
    OSPF_GBL_DEBUGFRR,
    OSPF_GBL_DEBUGPOLICY,
    OSPF_GBL_DEBUGERROR,
    OSPF_GBL_DEBUGDCN,
    OSPF_GBL_DEBUGALL, 
    OSPF_GBL_ROUTENUM, 
    OSPF_GBL_SPFRUN,
    OSPF_GBL_RESET,
    OSPF_GBL_CLRSTATISTIC,
    OSPF_GBL_CLRIFSTATISTIC,
    OSPF_GBL_RESTARTBEGIN,
    OSPF_GBL_NETWORKROUTECOUNT,
    OSPF_GBL_INTRAROUTECOUNT,
    OSPF_GBL_INTERROUTECOUNT,
    OSPF_GBL_EXTERNALROUTECOUNT,
    OSPF_GBL_WILDCARDROUTECOUNT,
    OSPF_GBL_ABRROUTECOUNT,
    OSPF_GBL_ASBRROUTECOUNT,
    OSPF_GBL_NETWORKROUTEPATHCOUNT,
    OSPF_GBL_INTRAROUTEPATHCOUNT,
    OSPF_GBL_INTERROUTEPATHCOUNT,
    OSPF_GBL_EXTERNALROUTEPATHCOUNT,
    OSPF_GBL_EXTERNALROUTENSSACOUNT,
    OSPF_GBL_ABRROUTEPATHCOUNT,
    OSPF_GBL_ASBRROUTEPATHCOUNT,
#ifdef HAVE_BFD
    OSPF_GBL_BFD,
    OSPF_GBL_BFD_MINRXINTR,
    OSPF_GBL_BFD_MINTXINTR,
    OSPF_GBL_BFD_DETMUL,
#endif
    OSPF_GBL_TYPE5COUNT,
    OSPF_GBL_TYPE11COUNT,
    OSPF_GBL_VRID,
    OSPF_GBL_INSTANCEID,
    OSPF_GBL_VPN,
    OSPF_GBL_VALIDHOPS,
    OSPF_GBL_ROUTETAG,
    OSPF_GBL_DOMAIN,
    OSPF_GBL_LOOPPREVENT,
    OSPF_GBL_DEFROUTEADV,
    OSPF_GBL_DCNLSA,
    OSPF_GBL_SYNPACKET,
    OSPF_GBL_DEFROUTECOST,
    OSPF_GBL_DEFROUTECOSTTYPE,
    OSPF_GLB_SWVERSION,

	OSPF_GLB_DEBUG_ON,
	OSPF_GLB_DEBUG_CLR,

	OSPF_GBL_MAXECMP,
	OSPF_GBL_NEXTHOPWEIGHT,
	OSPF_GBL_PACKET_BLOCK,
    OSPF_GLB_PREFERENCE,
    OSPF_GLB_ASE_PREFERENCE,
    OSPF_GBL_DESCRIPTION,
    OSPF_GLB_MPLS_ID,
    OSPF_GLB_MPLS_ID_ADV,
    OSPF_GLB_MPLS_ID_COST,
    OSPF_GLB_MPLS_TE,
	OSPF_GBL_ROUTERID_SETFLG,
	OSPF_GBL_NEW_ROUTERID,
	OSPF_GBL_AREACOUNT,
	OSPF_GBL_SYSLOG_DEBUG_ON,
	/*end*/
};
enum
{
   OSPF_PROCESS_DEBUG,
#ifdef OSPF_FRR
   OSPF_PROCESS_FRR,
   OSPF_PROCESS_FRR_NOLOOP
#endif
};
enum {
      OSPF_STUB_NOADV = 1,
      OSPF_STUB_ADV  
};
enum {
      OSPF_TRANSLATOR_ALWAYS = 1,
      OSPF_TRANSLATOR_CANDIDATE
};
enum {
      OSPF_TRANSLATOR_STATE_ENABLE = 1,
      OSPF_TRANSLATOR_STATE_ELECTED,
      OSPF_TRANSLATOR_STATE_DISABLE
};
enum {
      OSPF_DEFROUTE_ADV = 1,
      OSPF_DEFROUTE_NOADV  
};
/*OSPF Area Table
    ospfAreaId 
    ospfAuthType 
    ospfImportAsExtern {
                      importExternal (1),
                      importNoExternal (2),
                      importNssa (3)
                      }
  ospfSpfRuns 
  ospfAreaBdrRtrCount 
  ospfAsBdrRtrCount 
  ospfAreaLsaCount
  ospfAreaLsaCksumSum 
  ospfAreaSummary {
                       noAreaSummary (1),
                       sendAreaSummary (2)
                       }
  ospfAreaStatus
  ospfAreaNssaTranslatorRole { always (1), candidate (2) }
  ospfAreaNssaTranslatorState { enabled (1),
                       elected (2),
                       disabled (3)
                       }
  ospfAreaNssaTranslatorStabilityInterval 
  ospfAreaNssaTranslatorEvents 
*/
enum{
        OSPF_AREA_ID ,/*ospfAreaId*/
        OSPF_AREA_AUTHTYPE,/*ospfAuthType*/
        OSPF_AREA_AUTHDIS,/*ospfAuthDisplay*/
        OSPF_AREA_IMPORTASEXTERNAL,/*ospfImportAsExtern {importExternal (1), importNoExternal (2), importNssa (3)*/
        OSPF_AREA_SPFRUNNING, /*ospfSpfRuns*/
        OSPF_AREA_ABRCOUNT,/*ospfAreaBdrRtrCount*/
        OSPF_AREA_ASBRCOUNT,/*ospfAsBdrRtrCount*/
        OSPF_AREA_LSACOUNT,/*ospfAreaLsaCount*/
        OSPF_AREA_LSACHECKSUM,/*ospfAreaLsaCksumSum*/
        OSPF_AREA_SUMMARY,/*ospfAreaSummary */
        OSPF_AREA_STATUS,/*ospfAreaStatus*/
        OSPF_AREA_TRANSLATORROLE,/*ospfAreaNssaTranslatorRole*/
        OSPF_AREA_TRANSLATORSTATE,/*ospfAreaNssaTranslatorState*/ 
        OSPF_AREA_TRANSLATORSTABLEINTERVAL,/*ospfAreaNssaTranslatorStabilityInterval */
        OSPF_AREA_TRANSLATOREVENT,/*ospfAreaNssaTranslatorEvents */
        OSPF_AREA_TEENABLE,
        OSPF_AREA_NSSADEFAULTCOST,
        OSPF_AREA_AUTHKEYID ,
        OSPF_AREA_AUTHKEY,
        OSPF_AREA_CIPHERKEY,
        OSPF_AREA_EACHLSACOUNT,
        OSPF_AREA_DESCRIPTION
};

typedef struct {
   u_int area_id;
   
   u_int process_id;   
}tOSPF_AREA_INDEX;

enum {
    OSPF_AREA_IMPORT_EXTERNAL = 1,
    OSPF_AREA_IMPORT_STUB,
    OSPF_AREA_IMPORT_NSSA
};

enum {
    OSPF_NO_AREASUMMARY = 1, 
    OSPF_SEND_AREASUMMARY = 2 
};

enum {
    OSPF_TRANSLATOR_ROLE_ALWAYS = 1,
    OSPF_TRANSLATOR_ROLE_CANDIDATE
};

enum {
    OSPF_TRANSLATOR_ENABLE = 1,
    OSPF_TRANSLATOR_ELECTED,
    OSPF_TRANSLATOR_DISABLED
};

/*OSPF Area Default Metric Table
     INDEX { ospfStubAreaId, ospfStubTOS }

     ospfStubAreaId 
     ospfStubTOS 
     ospfStubMetric 
     ospfStubStatus 
     ospfStubMetricType {
                       ospfMetric (1),  -- OSPF Metric
                       comparableCost (2), -- external type 1
                       nonComparable  (3) -- external type 2
                       }
*/
enum {
    OSPF_STUBAREA_ID , 
    OSPF_STUBAREA_TOS ,
    OSPF_STUBAREA_METRICS, 
    OSPF_STUBAREA_STATUS, 
    OSPF_STUBAREA_METRICTYPE  
   };

enum {
    OSPF_STUB_METRICTYPE_INTERNAL = 1,
    OSPF_STUB_METRICTYPE_EXTERNAL1,
    OSPF_STUB_METRICTYPE_EXTERNAL2
};

/*stub area*/
typedef struct {
    u_int area;
    
    u_int tos;
    
    u_int process_id;
}tOSPF_STUBAREA_INDEX;

/*OSPF Link State Database
INDEX { ospfLsdbAreaId, ospfLsdbType,
               ospfLsdbLsid, ospfLsdbRouterId }

          ospfLsdbAreaId
          ospfLsdbType
          ospfLsdbLsid
          ospfLsdbRouterId
          ospfLsdbSequence
          ospfLsdbAge
          ospfLsdbChecksum
          ospfLsdbAdvertisement*/
enum {
        OSPF_LSDB_AREAID, 
        OSPF_LSDB_TYPE, 
        OSPF_LSDB_LSID, 
        OSPF_LSDB_ROUTERID, 
        OSPF_LSDB_SEQUENCE, 
        OSPF_LSDB_AGE, 
        OSPF_LSDB_CHECKSUM, 
        OSPF_LSDB_ADVERTISEMENT ,
        OSPF_LSDB_LEN,
        OSPF_LSDB_METRIC,
        OSPF_LSDB_OPTIONS,
        OSPF_LSDB_LINKID,
        OSPF_LSDB_ADVID,
        OSPF_LSDB_LINKTYPE,
        OSPF_LSDB_LINKCOUNT,
        OSPF_LSDB_NETROUTER,
        OSPF_LSDB_TOS,
        OSPF_LSDB_TAG,
        OSPF_LSDB_FADADDR,
        OSPF_LSDB_MASK,
        OSPF_LSDB_ETYPE
      };

/*lsa for all*/
typedef struct {
   u_int process_id;
   
   u_int area;
   
   u_int type;
   
   u_int id;
   
   u_int adv;
}tOSPF_LSDB_INDEX;


/*Address Range Table
  INDEX { ospfAreaRangeAreaId, ospfAreaRangeNet }
  ospfAreaRangeAreaId 
  ospfAreaRangeNet 
  ospfAreaRangeMask 
  ospfAreaRangeStatus
  ospfAreaRangeEffect  {advertiseMatching (1), doNotAdvertiseMatching (2)}
*/
enum {
        OSPF_ADDRRANGE_AREAID,
        OSPF_ADDRRANGE_NET,
        OSPF_ADDRRANGE_MASK,
        OSPF_ADDRRANGE_STATUS, 
        OSPF_ADDRRANGE_EFFECT 
};

/*area range*/
typedef struct {
    u_int process_id;
    
    u_int area;
    
    u_int dest;
    
    u_int mask;
}tOSPF_RANGE_INDEX;
typedef struct {
    u_int cmd_id;
    u_char cmdstr[30];
}stOSPF_DEBUG_CMD;

enum { 
    OSPF_MATCH_ADV = 1,  
    OSPF_MATCH_NOADV = 2
}  ;

/* OSPF Host Table
INDEX { ospfHostIpAddress, ospfHostTOS }

  ospfHostIpAddress 
  ospfHostTOS 
  ospfHostMetric 
  ospfHostStatus 
  ospfHostAreaID 
  ospfHostCfgAreaID */
enum {
             OSPF_HOST_IPADDR,
             OSPF_HOST_TOS,
             OSPF_HOST_METRIC,
             OSPF_HOST_AREAID,
             OSPF_HOST_CFGAREAID
};

/*OSPF Interface Table
   INDEX { ospfIfIpAddress, ospfAddressLessIf }
  ospfIfIpAddress 
  ospfAddressLessIf 
  ospfIfAreaId 
  ospfIfType {broadcast (1), nbma (2), pointToPoint (3),  pointToMultipoint (5)}
  ospfIfAdminStat 
  ospfIfRtrPriority
  ospfIfTransitDelay 
  ospfIfRetransInterval 
  ospfIfHelloInterval 
  ospfIfRtrDeadInterval 
  ospfIfPollInterval 
  ospfIfState {down (1), loopback (2),waiting (3),pointToPoint (4),designatedRouter (5),backupDesignatedRouter (6),otherDesignatedRouter (7)}
  ospfIfDesignatedRouter 
  ospfIfBackupDesignatedRouter 
  ospfIfEvents 
  ospfIfAuthKey 
  ospfIfStatus 
  ospfIfMulticastForwarding {blocked (1), -- no multicast forwarding    multicast (2), -- using multicast address unicast (3) -- to each OSPF neighbor}
  ospfIfDemand  TruthValue
  ospfIfAuthType 
  ospfIfLsaCount 
  ospfIfLsaCksumSum 
  ospfIfDesignatedRouterId 
  ospfIfBackupDesignatedRouterId 
  */


typedef struct {
     u_int process_id;
     
     u_int ipaddr;
     
     u_int addrlessif;
}tOSPF_IFINDEX;

/*interface type*/
enum
{
        OSPF_IFT_BCAST = 1,
        OSPF_IFT_NBMA , 
        OSPF_IFT_PPP , 
        OSPF_IFT_VLINK  ,
        OSPF_IFT_P2MP ,
        OSPF_IFT_SHAMLINK
};

/*ISM state*/
enum
{
        OSPF_IFS_DOWN  = 1, 
        OSPF_IFS_LOOPBACK , 
        OSPF_IFS_WAITING , 
        OSPF_IFS_PPP , 
        OSPF_IFS_DR ,
        OSPF_IFS_BDR , 
        OSPF_IFS_DROTHER 
};

enum {
     OSPF_MCAST_BLOCK = 1,
     OSPF_MCAST_NO_FOWWARDING,
     OSPF_MCAST_FOWWARDING,
     OSPF_MCAST_UNICAST
};

/*OSPF Interface Metric Table
INDEX { ospfIfMetricIpAddress,
          ospfIfMetricAddressLessIf,
          ospfIfMetricTOS }

  ospfIfMetricIpAddress 
  ospfIfMetricAddressLessIf 
  ospfIfMetricTOS
  ospfIfMetricValue 
  ospfIfMetricStatus */
enum {
    OSPF_IFMETRIC_IPADDR , 
    OSPF_IFMETRIC_IPADDRLESSIF , 
    OSPF_IFMETRIC_TOS , 
    OSPF_IFMETRIC_VALUE, 
    OSPF_IFMETRIC_STATUS,
    OSPF_IFMERTRIC_COSTSTATUS
    };

/*interface metric*/
typedef struct {
   u_int process_id;
   
   u_int ifip;
   
   u_int ifindex;
   
   u_int tos;
}tOSPF_IFMETRIC_INDEX;

/* OSPF Virtual Interface Table
  INDEX { ospfVirtIfAreaId, ospfVirtIfNeighbor }

  ospfVirtIfAreaId 
  ospfVirtIfNeighbor 
  ospfVirtIfTransitDelay 
  ospfVirtIfRetransInterval 
  ospfVirtIfHelloInterval 
  ospfVirtIfRtrDeadInterval 
  ospfVirtIfState  {
                       down (1), -- these use the same encoding
                       pointToPoint (4) -- as the ospfIfTable
                       }
  ospfVirtIfEvents 
  ospfVirtIfAuthKey 
  ospfVirtIfStatus 
  ospfVirtIfAuthType 
  ospfVirtIfLsaCount 
  ospfVirtIfLsaCksumSum 
  */
enum {
    OSPF_VIF_AREAID,
    OSPF_VIF_NEIGHBOR, 
    OSPF_VIF_TRANSIT_DELAY , 
    OSPF_VIF_RETRANSMITINTERVAL, 
    OSPF_VIF_HELLOINTERVAL, 
    OSPF_VIF_DEADINTERVAL, 
    OSPF_VIF_STATE, 
    OSPF_VIF_EVENT, 
    OSPF_VIF_AUTHKEY, 
    OSPF_VIF_STATUS,
    OSPF_VIF_AUTHTYPE , 
    OSPF_VIF_LSACOUNT,
    OSPF_VIF_LSACHECKSUM,
    OSPF_VIF_AUTHKEYID,
    OSPF_VIF_NBRSTATE,
    OSPF_VIF_TYPE,
    OSPF_VIF_ADDR,
    OSPF_VIF_NAME,
    OSPF_VIF_COST	
    };


/*virtual interface*/
typedef struct {
   u_int process_id;
   
   u_int area;
   
   u_int nbr;
} tOSPF_VIF_INDEX;

/*OSPF Neighbor Table
INDEX { ospfNbrIpAddr, ospfNbrAddressLessIndex }

  ospfNbrIpAddr 
   ospfNbrAddressLessIndex 
  ospfNbrRtrId 
  ospfNbrOptions 
   ospfNbrPriority
   ospfNbrState {down (1),attempt (2),init (3),twoWay (4),exchangeStart (5),exchange (6), loading (7),full (8)}
  ospfNbrEvents 
  ospfNbrLsRetransQLen 
  ospfNbmaNbrStatus 

  ospfNbmaNbrPermanence {dynamic (1), -- learned through protocol permanent (2) -- configured address}
   ospfNbrHelloSuppressed 
  ospfNbrRestartHelperStatus { notHelping (1),helping (2)}
  ospfNbrRestartHelperAge 

  ospfNbrRestartHelperExitReason { none (1),           -- not attempted
                              inProgress (2),     -- restart inprogress
                              completed (3),      -- successfullycompleted
                              timedOut (4),       -- timed out
                              topologyChanged (5) -- aborted due totopology change.
                            }
*/

typedef struct {
    u_int process_id;
    
    u_int ipaddr;
    
    u_int addrlessif;
}tOSPF_NBR_INDEX;

enum {  
    OSPF_NBMA_DYNMAIC = 1,
    OSPF_NBMA_PERMANENT = 2,
};

/*NSM state*/
enum
{
        OSPF_NS_DOWN = 1, 
        OSPF_NS_ATTEMPT , 
        OSPF_NS_INIT , 
        OSPF_NS_2WAY , 
        OSPF_NS_EXSTART , 
        OSPF_NS_EXCHANGE , 
        OSPF_NS_LOADING , 
        OSPF_NS_FULL,
        OSPF_NS_RESTART,
        OSPF_NS_MAX
};
/*neighbor state*/
enum
{
        EwriOspfNbrState_down = 1, 
        EwriOspfNbrState_attempt , 
        EwriOspfNbrState_init , 
        EwriOspfNbrState_twoWay , 
        EwriOspfNbrState_exchangeStart , 
        EwriOspfNbrState_exchange , 
        EwriOspfNbrState_loading , 
        EwriOspfNbrState_full,

};

/*OSPF Virtual Neighbor Table
  ospfVirtNbrArea 
  ospfVirtNbrRtrId 
  ospfVirtNbrIpAddr 
  ospfVirtNbrOptions 
   ospfVirtNbrState 
  ospfVirtNbrEvents 
  ospfVirtNbrLsRetransQLen 
  ospfVirtNbrHelloSuppressed 
  ospfVirtNbrRestartHelperStatus 
  ospfVirtNbrRestartHelperAge 
  ospfVirtNbrRestartHelperExitReason 
*/
enum {
    OSPF_VNBR_AREAID, 
    OSPF_VNBR_ROUTERID, 
    OSPF_VNBR_IPADDR, 
    OSPF_VNBR_OPTION , 
    OSPF_VNBR_STATE  ,
    OSPF_VNBR_EVENT, 
    OSPF_VNBR_RETRANSQLEN, 
    OSPF_VNBR_HELLOSUPRESS ,
    OSPF_VNBR_RESTARTHELPERSTATUS,
    OSPF_VNBR_RESTARTHELPERAGE,
    OSPF_VNBR_RESTARTHELPEREXITREASON
    };

typedef struct {
   u_int process_id ;
   
   u_int area;
   
   u_int nbr;
} tOSPF_VNBR_INDEX;

/*OSPF Link State Database, External

  ospfExtLsdbType
  ospfExtLsdbLsid 
  ospfExtLsdbRouterId 
  ospfExtLsdbSequence 
  ospfExtLsdbAge 
  ospfExtLsdbChecksum 
  ospfExtLsdbAdvertisement */
  enum {
    OSPF_EXTLSDB_TYPE, 
    OSPF_EXTLSDB_LSID, 
    OSPF_EXTLSDB_ROUTERID , 
    OSPF_EXTLSDB_SEQUENCE , 
    OSPF_EXTLSDB_AGE, 
    OSPF_EXTLSDB_CHECKSUM, 
    OSPF_EXTLSDB_ADVERTISEMENT,
    OSPF_EXTLSDB_METRIC,
    OSPF_EXTLSDB_NETMASK,
    OSPF_EXTLSDB_TOS,
    OSPF_EXTLSDB_FWADDR,
    OSPF_EXTLSDB_TAG,
    OSPF_EXTLSDB_OPTIONS,
    OSPF_EXTLSDB_ETYPE
    };

typedef struct {
   u_int process_id;
    
   u_int type;
   
   u_int id;
   
   u_int adv;
}tOSPF_EXTLSDB_INDEX;

/* ospfAreaAggregateTable OBJECT-TYPE
   INDEX { ospfAreaAggregateAreaID, ospfAreaAggregateLsdbType,
          ospfAreaAggregateNet, ospfAreaAggregateMask }

   ospfAreaAggregateAreaID  
   spfAreaAggregateLsdbType  {summaryLink (3),nssaExternalLink (7)}
   ospfAreaAggregateNet  
   ospfAreaAggregateMask  
   ospfAreaAggregateStatus  
   ospfAreaAggregateEffect  {advertiseMatching (1), doNotAdvertiseMatching (2)}
   ospfAreaAggregateExtRouteTag 
   */
   enum {
              OSPF_AREAAGGR_AREAID,
              OSPF_AREAAGGR_LSDBTYPE,
              OSPF_AREAAGGR_NET,
              OSPF_AREAAGGR_MASK,
              OSPF_AREAAGGR_STATUS,
              OSPF_AREAAGGR_EFFECT,
              OSPF_AREAAGGR_EXTROUTETAG
   };

typedef struct {
       u_int process_id;
       
       u_int area;
       
       u_int lstype;
       
       u_int net;
       
       u_int mask;
}tOSPF_AREAAGGR_INDEX;

/* OSPF Link State Database, link-local for non-virtual links
INDEX { ospfLocalLsdbIpAddress, ospfLocalLsdbAddressLessIf,
          ospfLocalLsdbType, ospfLocalLsdbLsid, ospfLocalLsdbRouterId
          }

  ospfLocalLsdbIpAddress 
  ospfLocalLsdbAddressLessIf
  ospfLocalLsdbType 
  ospfLocalLsdbLsid 
  ospfLocalLsdbRouterId 
  ospfLocalLsdbSequence
  ospfLocalLsdbAge 
  ospfLocalLsdbChecksum 
  ospfLocalLsdbAdvertisement */
enum {
            OSPF_IFLSDB_IPADDR,
            OSPF_IFLSDB_IPADDRLESSIF,    
            OSPF_IFLSDB_TYPE,
            OSPF_IFLSDB_LSID,
            OSPF_IFLSDB_ROUTERID,
            OSPF_IFLSDB_SEQUENCE,
            OSPF_IFLSDB_AGE,
            OSPF_IFLSDB_CHECKSUM,
            OSPF_IFLSDB_ADVERTISEMENT
};
typedef struct {
   u_int process_id;
   
   u_int ifaddr;
   
   u_int addrlessif;
   
   u_int type;
   
   u_int id;
   
   u_int adv;
}tOSPF_IFLSDB_INDEX;

/*OSPF Link State Database, link-local for virtual Links
INDEX { ospfVirtLocalLsdbTransitArea,
          ospfVirtLocalLsdbNeighbor,
          ospfVirtLocalLsdbType,
          ospfVirtLocalLsdbLsid,
          ospfVirtLocalLsdbRouterId
          }

  ospfVirtLocalLsdbTransitArea 
  ospfVirtLocalLsdbNeighbor 
  ospfVirtLocalLsdbType 
  ospfVirtLocalLsdbLsid 
  ospfVirtLocalLsdbRouterId 
  ospfVirtLocalLsdbSequence 
  ospfVirtLocalLsdbAge 
  ospfVirtLocalLsdbChecksum 
  ospfVirtLocalLsdbAdvertisement */
enum {
            OSPF_VIFLSDB_TRANSITAREA,
            OSPF_VIFLSDB_NEIGHBOR,    
            OSPF_VIFLSDB_TYPE,
            OSPF_VIFLSDB_LSID,
            OSPF_VIFLSDB_ROUTERID,
            OSPF_VIFLSDB_SEQUENCE,
            OSPF_VIFLSDB_AGE,
            OSPF_VIFLSDB_CHECKSUM,
            OSPF_VIFLSDB_ADVERTISEMENT
};
typedef struct {
   u_int process_id;
   
   u_int area;
   
   u_int nbr;
   
   u_int type;
   
   u_int id;
   
   u_int adv;
}tOSPF_VIFLSDB_INDEX;

/* OSPF Link State Database, AS-scope
INDEX { ospfAsLsdbType, ospfAsLsdbLsid, ospfAsLsdbRouterId }

      ospfAsLsdbType {asExternalLink (5),asOpaqueLink   (11)}
     ospfAsLsdbLsid 
     ospfAsLsdbRouterId 
     ospfAsLsdbSequence 
     ospfAsLsdbAge 
     ospfAsLsdbChecksum 
     ospfAsLsdbAdvertisement */
     enum {
            OSPF_ASLSDB_TYPE,
            OSPF_ASLSDB_LSID,
            OSPF_ASLSDB_ROUTERID,
            OSPF_ASLSDB_SEQUENCE,
            OSPF_ASLSDB_AGE,
            OSPF_ASLSDB_CHECKSUM,
            OSPF_ASLSDB_ADVERTISEMENT
};
typedef struct {
   u_int process_id;
   
   u_int type;
   
   u_int id;
   
   u_int adv;
}tOSPF_ASLSDB_INDEX;

 /*OSPF Area LSA Counter Table
INDEX { ospfAreaLsaCountAreaId, ospfAreaLsaCountLsaType }

      ospfAreaLsaCountAreaId 
      ospfAreaLsaCountLsaType 
      ospfAreaLsaCountNumber */
enum {
         OSPF_AREALSACOUNT_AREAID,
         OSPF_AREALSACOUNT_LSTYPE,
         OSPF_AREALSACOUNT_NUMBER
    };
 
typedef struct {
   u_int process_id;
   
   u_int area;
   
   u_int type;
}tOSPF_LSDBCOUNT_INDEX;

/*network table*/
enum {
    OSPF_NETWORK_AREAID,
    OSPF_NETWORK_DEST,        
    OSPF_NETWORK_MASK,
    OSPF_NETWORK_STATUS,  
    OSPF_NETWORK_AREA_STATUS    

};

typedef struct {
    u_int process_id;
    
   u_int area;
   
   u_int mask;
   
   u_int network;
}tOSPF_NETWORK_INDEX;

/*redistribute control*/
enum {
        OSPF_DISTRIBUTE_PROTO,
        OSPF_DISTRIBUTE_DEST,
        OSPF_DISTRIBUTE_MASK,
        OSPF_DISTRIBUTE_COST, 
        OSPF_DISTRIBUTE_COSTTYPE,
        OSPF_DISTRIBUTE_ADVERTISE,
        OSPF_DISTRIBUTE_STATUS,
        OSPF_DISTRIBUTE_TRANSLATE,
        OSPF_DISTRIBUTE_TAG
    };

enum{
    OSPF_DISTRIBUTERANGE_PROTO,
    OSPF_DISTRIBUTERANGE_DEST,
    OSPF_DISTRIBUTERANGE_MASK,
    OSPF_DISTRIBUTERANGE_STATUS,
    OSPF_DISTRIBUTERANGE_TRANSLATE
};
typedef struct {
   u_int process_id;
   
   u_int proto;

   u_int proto_process;
   
   u_int mask;
   
   u_int dest;
}tOSPF_DISTRIBUTE_INDEX;


typedef struct {
    u_int uiProcessId;

    u_int uiState;
}tOSPF_DISTRIBUTE_PROCESS;


typedef struct {
   u_int process_id;
   
   u_int type; 
   
   u_int mask;
   
   u_int dest;
   
   u_int areaid;

   u_int nexthop;
}tOSPF_ROUTE_INDEX;


typedef struct {
   u_int process_id;
   
   u_int areaid;

   u_int dest;
   
   u_int mask;
   
   u_int type;
   
   u_int nexthop;
   
   u_int uint;	/*ifindex*/
      
   u_int cost;
      
   u_int priority;
   
   u_int age;

   u_int bk_uint;	/*bk ifindex*/
   
   u_int bk_nexthop;

}stOSPF_ROUTE_NET;



/*sham link*/
typedef struct {
   u_int process_id;
   
   u_int if_addr;
   
   u_int nbr_addr;
} tOSPF_SHAMLINK_INDEX;

enum {
    OSPF_SHAMLINK_STATUS,
    OSPF_SHAMLINK_AREAID,
    OSPF_SHAMLINK_TRANSIT_DELAY , 
    OSPF_SHAMLINK_RETRANSMITINTERVAL, 
    OSPF_SHAMLINK_HELLOINTERVAL, 
    OSPF_SHAMLINK_DEADINTERVAL, 
    OSPF_SHAMLINK_AUTHKEY,    
    OSPF_SHAMLINK_AUTHTYPE , 
    OSPF_SHAMLINK_AUTHKEYID,
    OSPF_SHAMLINK_COST
    };

/*private nssa range,index is same as rangexxx*/
enum {
   OSPF_NSSARANGE_AREAID,
   OSPF_NSSARANGE_DEST,    
   OSPF_NSSARANGE_MASK,   
   OSPF_NSSARANGE_STATUS ,
   OSPF_NSSARANGE_COST ,
   OSPF_NSSARANGE_EFFECT 
};

enum {
   OSPF_ASBRRANGE_AREAID,
   OSPF_ASBRRANGE_DEST,    
   OSPF_ASBRRANGE_MASK,   
   OSPF_ASBRRANGE_STATUS ,
   OSPF_ASBRRANGE_COST ,
   OSPF_ASBRRANGE_EFFECT ,
   OSPF_ASBRRANGE_MATCH,
   OSPF_ASBRRANGE_COUNT
};
/*end*/

/*TRAP
ospfConfigErrorType OBJECT-TYPE
          SYNTAX       INTEGER {
                          badVersion (1),
                          areaMismatch (2),
                          unknownNbmaNbr (3), -- Router is DR eligible
                          unknownVirtualNbr (4),
                          authTypeMismatch(5),
                          authFailure (6),
                          netMaskMismatch (7),
                          helloIntervalMismatch (8),
                          deadIntervalMismatch (9),
                          optionMismatch (10),
                          mtuMismatch (11),
                          duplicateRouterId (12),
                          noError (13) }

*/
/*packet type*/
enum
{
        OSPF_PACKET_HELLO       = 0x01,
        OSPF_PACKET_DBD        = 0x02,
        OSPF_PACKET_REQUEST = 0x03,
        OSPF_PACKET_UPDATE = 0x04,
        OSPF_PACKET_ACK = 0x05
};

/* Link State Types */
enum
{
        OSPF_LS_ROUTER = 0x01,
        OSPF_LS_NETWORK = 0x02,
        OSPF_LS_SUMMARY_NETWORK = 0x03, 
        OSPF_LS_SUMMARY_ASBR = 0x04,
        OSPF_LS_AS_EXTERNAL = 0x05,
        OSPF_LS_MULTICAST = 0x06,
        OSPF_LS_TYPE_7 = 0x07,
        OSPF_LS_TYPE_8 = 0x08,
        OSPF_LS_TYPE_9 = 0x09,
        OSPF_LS_TYPE_10 = 0x0a,
        OSPF_LS_TYPE_11 = 0x0b,
        OSPF_LS_MAX = 0x0c
};


/*route dest type*/
enum
{
        OSPF_ROUTE_ASBR = 0,  
        OSPF_ROUTE_ABR ,
        OSPF_ROUTE_NETWORK
};

/*route path type*/
enum
{
        OSPF_PATH_INVALID = 0x00,
        OSPF_PATH_INTRA,
        OSPF_PATH_INTER,
        OSPF_PATH_ASE,
        OSPF_PATH_ASE2
};

/* Configuration Error Types of IF_CONFIG_ ERROR_TRAP */
enum {
              ERROR_VERSION = 1, 
              ERROR_AREA, 
              ERROR_UNKOWN_NBMANBR, 
              ERROR_UNKOWN_VNBR, 
              ERROR_AUTH_TYPE, 
              ERROR_AUTH,  
              ERROR_NETMASK, 
              ERROR_HELLO_TIME, 
              ERROR_DEAD_TIME, 
              ERROR_OPTION ,
              ERROR_MTU,
              ERROR_ROUTERID,
              ERROR_NO
};



/*route policy */
enum {      
      OSPF_ROUTEPOLICY_STATUS,
      OSPF_ROUTEPOLICY_TYPE,
      OSPF_ROUTEPOLICY_MAPNAME,
};

typedef struct {   
   u_int process_id;
   
   u_int proto; 

   u_int proto_process; 
   
   u_int policy_index;
}tOSPF_REDISPOLICY_INDEX;

typedef struct {   
   u_int process_id;
   
   u_int policy_index;   
}tOSPF_FILTERPOLICY_INDEX;


#define       OSPF_GR_LSID 0x03000000

#define       OSPF_DCN_LSID 0xcaffee00
#define       OSPF_TE_LSID  0x01000000

/*hello interval on interface*/
#define        OSPF_DEFAULT_HELLO_INTERVAL 10

/*transmit delay for interface*/
#define        OSPF_DEFAULT_TRANSMIT_DELAY        1

/*dead interval*/
#define        OSPF_DEFAULT_ROUTER_DEAD_INTERVAL        40

#define        OSPF_DEFAULT_VLINK_DEAD_INTERVAL           40

/*retransmit interval*/
#define        OSPF_DEFAULT_RETRANSMIT_INTERVAL        5  

/*interface poll interval*/
#define        OSPF_DEFAULT_POLL_INTERVAL         120
/*default dr priority*/
#define        OSPF_DEFAULT_PRIORITY        1

/*mcast support flag*/
#define        OSPF_DEFAULT_MCAST_SUPPORT_VALUE 3

/*demand circuit support flag*/
#define        OSPF_DEFAULT_DC_SUPPORT_VALUE 1 

/*primary domian-id*/
#define OSPF_PRIMARY_DOAMIN  1
#define OSPF_SECONDRY_DOAMIN  0

#define OSPF_DEFAULT_HOLD_DOWN_TIME     10
#define OSPF_DEFAULT_HOLD_COST_TIME     10

#define OSPF_DEFAULT_STUB_ROUTER_TIME   500

#ifndef STATUS
#define STATUS    int     
#endif

/*RFC 1765*/
#define       OSPF_DEFAULT_ASE_LIMIT ((u_int)-1)
#define       OSPF_DEFAULT_OVERFLOW_INERVAL 0 


enum {
    OSPF_UTIL_GLOBAL,
    OSPF_UTIL_INTERFACE
    };

/*OSPF Sync State */
enum {
    OSPF_LDP_UNENABLE = 0,
    OSPF_LDP_INIT ,
    OSPF_LDP_HOLD_DOWN,
    OSPF_LDP_HOLD_MAX_COST,
    OSPF_LDP_HOLD_NORMAL_COST,
    OSPF_LDP_SYNC_ACHIEVED,
    };

enum {
    OSPF_SYN_MSG,  
    OSPF_LDP_MSG,
    OSPF_BFD_MSG,
    OSPF_IFLINK_MSG,
};

enum {
    OSPF_BFD_BIND_MSG,
    OSPF_BFD_UNBIND_MSG,
    OSPF_BFD_MOD_MSG,
};

/*function declare*/
STATUS ospfGetApi(u_int index,u_int cmd, void *var);
STATUS ospfSetApi(u_int index, u_int cmd, void *var);
STATUS ospfSyncApi(u_int index, u_int cmd, void *var);
STATUS ospfInstanceGetFirst(u_int *p_id);
STATUS ospfInstanceGetNext(u_int instanceid, u_int *p_nextid);
STATUS ospfAreaGetFirst(void *index);
STATUS ospfAreaGetNext(void *index, void *p_nextindex);
STATUS ospfAreaGetApi(void *index, u_int cmd, void *var);
STATUS ospfAreaSetApi(void *p_index, u_int cmd,void *var);
STATUS ospfAreaSyncApi(void *p_index, u_int cmd, void *var);
STATUS ospfStubAreaGetFirst(tOSPF_STUBAREA_INDEX *p_index);
STATUS ospfStubAreaGetNext(tOSPF_STUBAREA_INDEX *p_index, tOSPF_STUBAREA_INDEX *p_nextindex);
STATUS ospfStubAreaGetApi(tOSPF_STUBAREA_INDEX *p_index, u_int cmd,void *var);
STATUS ospfStubAreaSetApi(tOSPF_STUBAREA_INDEX * p_index, u_int cmd, void * var);
STATUS ospfStubAreaSyncApi(tOSPF_STUBAREA_INDEX * p_index, u_int cmd, void * var);
STATUS ospfLsdbGetFirst(tOSPF_LSDB_INDEX *p_index);
STATUS ospfLsdbGetNext(tOSPF_LSDB_INDEX *p_index, tOSPF_LSDB_INDEX *p_nextindex);
STATUS ospfLsdbGetApi(tOSPF_LSDB_INDEX *p_index, u_int cmd, void *var);
STATUS ospfExtLsdbGetFirst(tOSPF_LSDB_INDEX *p_index);
STATUS ospfExtLsdbGetNext(tOSPF_LSDB_INDEX *p_index, tOSPF_LSDB_INDEX *p_nextindex);
STATUS ospfExtLsdbGetApi(tOSPF_LSDB_INDEX *p_index, u_int cmd, void *var);
STATUS ospfASLsdbGetFirst(tOSPF_ASLSDB_INDEX *p_index);
STATUS ospfASLsdbGetNext(tOSPF_ASLSDB_INDEX *p_index, tOSPF_ASLSDB_INDEX *p_nextindex);
STATUS ospfASLsdbGetApi(tOSPF_ASLSDB_INDEX *p_index, u_int cmd, void *var);
STATUS ospfIfLsdbGetFirst(tOSPF_IFLSDB_INDEX *p_index);
STATUS ospfIfLsdbGetNext(tOSPF_IFLSDB_INDEX *p_index, tOSPF_IFLSDB_INDEX *p_nextindex);
STATUS ospfIfLsdbGetApi(tOSPF_IFLSDB_INDEX *p_index, u_int cmd, void *var);
STATUS ospfVIfLsdbGetFirst(tOSPF_VIFLSDB_INDEX *p_index);
STATUS ospfVIfLsdbGetNext(tOSPF_VIFLSDB_INDEX *p_index, tOSPF_VIFLSDB_INDEX *p_nextindex);
STATUS ospfVIfLsdbGetApi(tOSPF_VIFLSDB_INDEX *p_index, u_int cmd, void *var);
STATUS ospfAreaLsaCountGetFirst(tOSPF_LSDBCOUNT_INDEX *p_index);
STATUS ospfAreaLsaCountGetNext(tOSPF_LSDBCOUNT_INDEX *p_index, tOSPF_LSDBCOUNT_INDEX *p_nextindex);
STATUS ospfAreaLsaCountGetApi( tOSPF_LSDBCOUNT_INDEX *p_index, u_int cmd, void *var);
STATUS ospfAreaRangeGetFirst(tOSPF_RANGE_INDEX *p_index);
STATUS ospfAreaRangeGetNext(tOSPF_RANGE_INDEX *p_index, tOSPF_RANGE_INDEX *p_nextindex);
STATUS ospfAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var );
STATUS ospfAreaRangeSyncApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var );
STATUS ospfAreaRangeGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfAreaAggregateGetFirst(tOSPF_AREAAGGR_INDEX *p_index);
STATUS ospfAreaAggregateGetNext(tOSPF_AREAAGGR_INDEX *p_index, tOSPF_AREAAGGR_INDEX *p_nextindex);
STATUS ospfAreaAggregateGetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd, void *var);
STATUS _ospfAreaAggregateSetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd, void *var, u_int flag);
STATUS ospfAreaAggregateSetApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd, void *var );
STATUS ospfAreaAggregateSyncApi(tOSPF_AREAAGGR_INDEX *p_index, u_int cmd, void *var );
STATUS ospfNssaAreaRangeGetFirst(tOSPF_RANGE_INDEX *p_index);
STATUS ospfNssaAreaRangeGetNext(tOSPF_RANGE_INDEX *p_index, tOSPF_RANGE_INDEX *p_nextindex);
STATUS ospfNssaAreaRangeSetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var );
STATUS ospfNssaAreaRangeSyncApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var );
STATUS ospfNssaAreaRangeGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfHostGetFirst(u_int *p_id);
STATUS ospfHostGetNext(u_int id, u_int * p_nextid);
STATUS ospHostGetApi(u_int id, u_int cmd, void *var);
STATUS ospHostSetApi(u_int id, u_int cmd, void *var);
STATUS ospfIfGetFirst(tOSPF_IFINDEX *p_index);
STATUS ospfIfGetNext(tOSPF_IFINDEX *p_index, tOSPF_IFINDEX *p_next_index);
STATUS ospfIfGetApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var);
STATUS ospfIfSetApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var);
STATUS ospfIfSynApi(tOSPF_IFINDEX *p_index, u_int cmd, void *var);
STATUS ospfIfMetricGetFirst(tOSPF_IFMETRIC_INDEX *p_index);
STATUS ospfIfMetricGetNext(tOSPF_IFMETRIC_INDEX *p_index, tOSPF_IFMETRIC_INDEX *p_nextindex);
STATUS ospfIfMetricGetApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var);
STATUS ospfIfMetricSetApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var);
STATUS ospfIfMetricSyncApi(tOSPF_IFMETRIC_INDEX *p_index, u_int cmd, void *var);
STATUS ospfVifGetFirst(tOSPF_VIF_INDEX *p_index);
STATUS ospfVifGetNext(tOSPF_VIF_INDEX *p_index, tOSPF_VIF_INDEX *p_nextindex);
STATUS ospfVifGetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var);
STATUS ospfVifSetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var);
STATUS ospfVifSyncApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNbrGetFirst(tOSPF_NBR_INDEX *p_index);
STATUS ospfNbrGetNext(tOSPF_NBR_INDEX *p_index, tOSPF_NBR_INDEX *p_next_index);
STATUS ospfNbrSetApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNbrSyncApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNbrGetApi(tOSPF_NBR_INDEX *p_index, u_int cmd, void *var);
STATUS ospfVnbrGetFirst(tOSPF_VIF_INDEX *p_index);
STATUS ospfVnbrGetNext(tOSPF_VIF_INDEX *p_index, tOSPF_VIF_INDEX *p_nextindex);
STATUS ospfVnbrGetApi(tOSPF_VIF_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNetworkGetFirst(tOSPF_NETWORK_INDEX *p_index);
STATUS ospfNetworkGetNext(tOSPF_NETWORK_INDEX *p_index, tOSPF_NETWORK_INDEX *p_nextindex);
STATUS ospfNetworkSetApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNetworkSyncApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNetworkGetApi(tOSPF_NETWORK_INDEX *p_index, u_int cmd, void *var);
STATUS ospfDistributeGetFirst(tOSPF_DISTRIBUTE_INDEX *p_index);
STATUS ospfDistributeGetNext(tOSPF_DISTRIBUTE_INDEX *p_index, tOSPF_DISTRIBUTE_INDEX *p_nextindex);
STATUS ospfDistributeSetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfDistributeSyncApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfDistributeGetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfNetwrokRouteGetFirst(tOSPF_ROUTE_INDEX *p_index);
STATUS ospfNetwrokRouteGetNext(tOSPF_ROUTE_INDEX *p_index, tOSPF_ROUTE_INDEX *p_nextindex);
STATUS ospfNetwrokRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospAbrRouteGetFirst(tOSPF_ROUTE_INDEX *p_index);
STATUS ospfAbrRouteGetNext(tOSPF_ROUTE_INDEX *p_index, tOSPF_ROUTE_INDEX *p_nextindex);
STATUS ospfAbrRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospAsbrRouteGetFirst(tOSPF_ROUTE_INDEX *p_index);
STATUS ospfAsbrRouteGetNext(tOSPF_ROUTE_INDEX *p_index, tOSPF_ROUTE_INDEX *p_nextindex);
STATUS ospfAsbrRouteGetApi(tOSPF_ROUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfRedisPolicyFirst(tOSPF_REDISPOLICY_INDEX *p_index);
STATUS ospfRedisPolicyGetNext(tOSPF_REDISPOLICY_INDEX *p_index, tOSPF_REDISPOLICY_INDEX *p_nextindex);
STATUS ospfRedisPolicySetApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd, void *var);
STATUS ospfRedisPolicySyncApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd, void *var);
STATUS ospfRedisPolicyGetApi(tOSPF_REDISPOLICY_INDEX *p_index, u_int cmd, void *var);
STATUS ospfFilterPolicyFirst(void *index);
STATUS ospfFilterPolicyGetNext(void * index, void * p_nextindex);
STATUS ospfFilterPolicySetApi(void  *index, u_int cmd, void *var);
STATUS ospfFilterPolicySyncApi(void  *index, u_int cmd, void *var);
STATUS ospfFilterPolicyGetApi(void  *index, u_int cmd, void *var);
STATUS ospfNbrGetSelect(tOSPF_NBR_INDEX *p_index, tOSPF_NBR_INDEX *p_next_index);
STATUS ospfRedisRangeGetFirst(tOSPF_DISTRIBUTE_INDEX *p_index);
STATUS ospfRedisRangeGetApi(tOSPF_DISTRIBUTE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfRedisRangeGetNext(tOSPF_DISTRIBUTE_INDEX *p_index, tOSPF_DISTRIBUTE_INDEX *p_nextindex);
STATUS ospfInstanceSetApi(u_int id, u_int cmd,  void *var);
STATUS ospfInstanceGetNext(u_int instanceid, u_int *p_nextid);
STATUS ospfInstanceGetApi(u_int id,u_int cmd, void *var);
STATUS ospfInstanceGetFirst(u_int *p_id);
STATUS ospfAsbrSummerySetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var );
STATUS ospfAsbrSummeryGetApi(tOSPF_RANGE_INDEX *p_index, u_int cmd, void *var);
STATUS ospfAsbrSummeryGetFirst(tOSPF_RANGE_INDEX *p_index);
STATUS ospfAsbrSummeryGetNext(tOSPF_RANGE_INDEX *p_index,tOSPF_RANGE_INDEX *p_nextindex);
STATUS ospfShamlinkGetFirst(tOSPF_SHAMLINK_INDEX *p_index);
STATUS ospfShamlinkGetApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var);
STATUS ospfShamlinkSetApi(tOSPF_SHAMLINK_INDEX *p_index, u_int cmd, void *var);
STATUS ospfShamlinkGetNext(tOSPF_SHAMLINK_INDEX *p_index, tOSPF_SHAMLINK_INDEX *p_nextindex);


#ifdef OSPF_MASTER_SLAVE_SYNC
STATUS ospf_master_slave_sync_recv(int iModid, u_int ulCmd, void *var, u_int uiIdxLen, u_int uiStrLen, u_int uiFlg);
#endif
void ospf_nexthopweight_add(struct ospf_process *p_process, tlv_t *octet);
int ospf_nexthopweight_get(struct ospf_process *p_process, tlv_t *octet);
void ospf_advertise_verify(struct ospf_asbr_range *p_rangeT5);
void ospf_asbr_nssa_verify(struct ospf_asbr_range *p_rangeT5);
void ospf_asbr_nssa_range_update_verify(struct ospf_asbr_range *p_rangeT5);
void ospf_stub_router_lsa_originate(struct ospf_process *p_process);
void ospf_refresh_stub_router_all(void);

void ospf_ldp_timer_start(struct ospf_if *p_if);
#ifdef HAVE_BFD
#include "bfd_nm.h"
BOOL ospf_if_bfd_for_static_Enable(u_long ulIfIndex);
void ospf_rtsock_bfd_msg_input(BFD_MSG_T *pstBfdMsg);
void ospf_if_bind_bfd_enable(struct ospf_if *pstIf);
#endif

int ospf_if_ldp_enable_lookup(u_long ulIfindex);


void ospf_syn_recv(tlv_t *p_octet);
STATUS mplsL3VpnGetRD(u_int *pName, u_int *pRD);


#define ospfvaule_to_nm(x) ((x == TRUE)? TRUE : FALSE)
#define nmvaule_to_ospf(x) ((x == TRUE)? TRUE : FALSE)

/*current configure applied instance id*/
 extern u_int ospf_config_instance;
#define ospf_current_instance() (ospf_process_lookup(&ospf, ospf_config_instance))
 
#if 1//def USP_MULTIINSTANCE_WANTED
#define ospf_get_nm_process(x) (ospf_process_lookup(&ospf, (x)->process_id)) 
#define ospf_nm_process_check(x) 
#define ospf_nm_process_id(x) ((x)->process_id)
#else
#define ospf_get_nm_process(x) ospf_current_instance()
#define ospf_nm_process_check(x) \
         if ((x)->process_id > ospf_config_instance) break;\
         else if ((x)->process_id != ospf_config_instance) continue;
#define ospf_nm_process_id(x) ospf_config_instance         
#endif

#define OSPF_NO_SET_VAL 0xFFFFFFFF


int ospf_ifindex_to_process(u_int ulIfunit, u_int *pulProcessId, u_int *pulAddr);

STATUS uspIfGetFirst(u_int *unit);
STATUS uspIfGetNext(u_int prev,u_int* next);
STATUS uspL3IfGetApi(u_int instanceRef, u_int cmd, void *var);

int interface_ospf_config_build(u_int uiIfIndex,struct vty *vty,u_long *pulLine);
STATUS ospfGetIfExistApi(u_int uiIfIndex, void *var);
STATUS ospfIfConfigLoadIpApi(u_int uiIfIndex, struct prefix_ipv4 stIpAddr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCospf_mib_helperh */
