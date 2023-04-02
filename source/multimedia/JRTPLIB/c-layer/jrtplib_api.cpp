#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpudpv6transmitter.h"
#include "rtpipv4address.h"
#include "rtpipv6address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtppacket.h"
#include "rtplibraryversion.h"

#include "rtpsourcedata.h"
#include "rtpabortdescriptors.h"
#include "rtpselect.h"
#include "rtprandom.h"


#include "jrtplib_api.h"


#ifdef __cplusplus
extern "C" {
#endif

struct jrtp_session_s
{
    int state;
    char local_address[64];
    uint16_t local_rtpport;
    uint16_t local_rtcpport;
    uint16_t rtpport;
    uint16_t rtcpport;
    char address[64];  
    int istcp;  
    int isipv6; 
    int payload;
    int clock;
    int framerate;
    int real_timespec;
    jrtplib::RTPSession jrtp_session;
};

jrtp_session_t * jrtp_session_create(void)
{
    jrtp_session_t *jrtpsess = (jrtp_session_t *)malloc(sizeof(jrtp_session_t));
    return jrtpsess;
}

int jrtp_session_destroy(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        jrtpsess->jrtp_session.BYEDestroy(jrtplib::RTPTime(10,0),0,0);
        //jrtpsess->jrtp_session.Destroy();
        free(jrtpsess);
    }
    return 0;
}

int jrtp_session_start(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        jrtplib::RTPUDPv4TransmissionParams transparams;
        jrtplib::RTPUDPv6TransmissionParams transparamsv6;
	    jrtplib::RTPSessionParams sessparams;
        jrtplib::RTPTransmitter::TransmissionProtocol proto;
        if(jrtpsess->istcp)
        {
            proto = jrtplib::RTPTransmitter::TCPProto;
        }
        else
        {
            proto = jrtplib::RTPTransmitter::IPv4UDPProto;
            if(jrtpsess->isipv6)
                proto = jrtplib::RTPTransmitter::IPv6UDPProto;
        }

	    sessparams.SetOwnTimestampUnit(1.0/jrtpsess->clock);		
	    sessparams.SetAcceptOwnPackets(true);

        if(jrtpsess->isipv6)
        {
            struct in6_addr addrv6;
            inet_pton (AF_INET6, jrtpsess->address, &addrv6);
            transparamsv6.SetPortbase(jrtpsess->local_rtpport);
            if(strlen((jrtpsess->local_address)))
                transparamsv6.SetBindIP(addrv6);

            if(jrtpsess->jrtp_session.Create(sessparams, &transparamsv6, proto) == 0)
            {
                inet_pton (AF_INET6, jrtpsess->address, &addrv6);
                jrtplib::RTPIPv6Address destaddr(addrv6.s6_addr, jrtpsess->rtpport);
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                    return -1;
            }  
        }
        else
        {
            transparams.SetPortbase(jrtpsess->local_rtpport);
            if(strlen((jrtpsess->local_address)))
                transparams.SetBindIP(ntohl(inet_addr(jrtpsess->local_address)));

            if(jrtpsess->jrtp_session.Create(sessparams, &transparams, proto) == 0)
            {
                jrtplib::RTPIPv4Address destaddr(ntohl(inet_addr(jrtpsess->address)), jrtpsess->rtpport, jrtpsess->rtcpport);
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                    return -1;
            }    
        }
        jrtpsess->jrtp_session.SetDefaultPayloadType(jrtpsess->payload);
        jrtpsess->jrtp_session.SetDefaultMark(false);
        if(jrtpsess->clock && jrtpsess->framerate)
        {    
            jrtpsess->jrtp_session.SetDefaultTimestampIncrement(jrtpsess->clock/jrtpsess->framerate);     
            //jrtpsess->real_clock  = jrtplib::RTPTime::CurrentTime().GetMicroSeconds() + 1000/jrtpsess->framerate;
        }     
        jrtpsess->state = 1;
        return 0;
    }
    return -1;
}

int jrtp_session_stop(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        jrtpsess->state = 0;
        return 0;
    }
    return -1;
}

int jrtp_session_destination(jrtp_session_t *jrtpsess, char *address, uint16_t rtpport, uint16_t rtcpport)
{
    if(jrtpsess)
    {
        memset(jrtpsess->address, 0, sizeof(jrtpsess->address));
        if(address)
        {
            if(strstr(address, "."))
                jrtpsess->isipv6 = 0;
            else if(strstr(address, ":"))
                jrtpsess->isipv6 = 1;
            strcpy(jrtpsess->address, address);
        }
        jrtpsess->rtpport = rtpport;
        jrtpsess->rtcpport = rtcpport;
        return 0;

    }
    return -1;
}

int jrtp_session_local_set(jrtp_session_t *jrtpsess, char *address, uint16_t rtpport, uint16_t rtcpport)
{
    if(jrtpsess)
    {
        memset(jrtpsess->local_address, 0, sizeof(jrtpsess->local_address));
        if(address)
            strcpy(jrtpsess->local_address, address);
        jrtpsess->local_rtpport = rtpport;
        jrtpsess->local_rtcpport = rtcpport;
        return 0;
    }
    return -1;
}

int jrtp_session_overtcp_set(jrtp_session_t *jrtpsess, int istcp)
{
    if(jrtpsess)
    {
        jrtpsess->istcp = istcp;		
        return 0;
    }
    return -1;
}

int jrtp_session_payload_set(jrtp_session_t *jrtpsess, int pt, int clock)
{
    if(jrtpsess)
    {
        jrtpsess->payload = pt;		
	    jrtpsess->clock = clock;
        return 0;
    }
    return -1;
}

int jrtp_session_framerate_set(jrtp_session_t *jrtpsess, int framerate)
{
    if(jrtpsess)
    {
        jrtpsess->framerate = framerate;		
        return 0;
    }
    return -1;
}

int jrtp_session_rtpsock(jrtp_session_t *jrtpsess)
{
    jrtplib::SocketType m_sockets = 0;
    jrtplib::RTPTransmissionInfo *pTrans = NULL;
    if (jrtpsess)
    {
        pTrans = jrtpsess->jrtp_session.GetTransmissionInfo();
        if (jrtpsess->isipv6 && pTrans)
        {
            jrtplib::RTPUDPv6TransmissionInfo *pInfov6 = static_cast<jrtplib::RTPUDPv6TransmissionInfo *>(pTrans);
            if (pInfov6)
                m_sockets = pInfov6->GetRTPSocket();
        }
        else if (pTrans)
        {
            jrtplib::RTPUDPv4TransmissionInfo *pInfo = static_cast<jrtplib::RTPUDPv4TransmissionInfo *>(pTrans);
            if (pInfo)
                m_sockets = pInfo->GetRTPSocket();
        }
    }
    return m_sockets;
}

int jrtp_session_rtcpsock(jrtp_session_t *jrtpsess)
{
    jrtplib::SocketType m_sockets = 0;
    jrtplib::RTPTransmissionInfo *pTrans = NULL;
    if (jrtpsess)
    {
        pTrans = jrtpsess->jrtp_session.GetTransmissionInfo();
        if (jrtpsess->isipv6 && pTrans)
        {
            jrtplib::RTPUDPv6TransmissionInfo *pInfov6 = static_cast<jrtplib::RTPUDPv6TransmissionInfo *>(pTrans);
            if (pInfov6)
                m_sockets = pInfov6->GetRTCPSocket();
        }
        else if (pTrans)
        {
            jrtplib::RTPUDPv4TransmissionInfo *pInfo = static_cast<jrtplib::RTPUDPv4TransmissionInfo *>(pTrans);
            if (pInfo)
                m_sockets = pInfo->GetRTCPSocket();
        }
    }
    return m_sockets;
}

int jrtp_session_rtcpdelay(jrtp_session_t *jrtpsess)
{
    if (jrtpsess)
    {
        return jrtpsess->jrtp_session.GetRTCPDelay().GetMicroSeconds();
    }
    return -1;
}

int jrtp_session_rtpdelay(jrtp_session_t *jrtpsess)
{
    if (jrtpsess)
    {
        int real_timespec = jrtplib::RTPTime::CurrentTime().GetMicroSeconds();
        real_timespec = jrtpsess->real_timespec - real_timespec;
        if(jrtpsess->framerate)
            jrtpsess->real_timespec += 1000/jrtpsess->framerate; 
        return real_timespec;
    }
    return -1;
}

int jrtp_session_rtprtcp_sched(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        int status = 0;
        status = jrtpsess->jrtp_session.Poll();
        return status;
    }
    return -1;
}

int jrtp_session_sendto(jrtp_session_t *jrtpsess, const void *data, size_t len,
	                uint8_t pt, bool mark, uint32_t timestampinc)
{
    if(jrtpsess)
    {
        int status = jrtpsess->jrtp_session.SendPacket(data, len, pt, mark, timestampinc);
        return status;
    }
    return -1;
}



int jrtp_session_recvfrom(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        int status = 0;
        //status = jrtpsess->jrtp_session.Poll();
		jrtpsess->jrtp_session.BeginDataAccess();
		// check incoming packets
		if (jrtpsess->jrtp_session.GotoFirstSourceWithData())
		{
			do
			{
				jrtplib::RTPPacket *pack;
				
				while ((pack = jrtpsess->jrtp_session.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					printf("Got packet !\n");
					//uint8_t *loaddata = pack->GetPayloadData();
					/*size_t len		 = pack->GetPayloadLength();
					if(pack->GetPayloadType() == 96) //H264
					{
						if(pack->HasMarker()) // the last packet
						{
							memcpy(&buff[pos],loaddata,len);	
							fwrite(buff, 1, pos+len, fd);
							pos = 0;
						}
						else
						{
							memcpy(&buff[pos],loaddata,len);
							pos = pos + len;	
						}
					}
                    else
					{
						printf("!!!  GetPayloadType = %d !!!! \n ",pack->GetPayloadType());
					}*/
					// we don't longer need the packet, so
					// we'll delete it
					jrtpsess->jrtp_session.DeletePacket(pack);
				}
			} while (jrtpsess->jrtp_session.GotoNextSourceWithData());
		}
		
		jrtpsess->jrtp_session.EndDataAccess();
        return status;
    }
    return -1;
}



#ifdef __cplusplus
};
#endif
