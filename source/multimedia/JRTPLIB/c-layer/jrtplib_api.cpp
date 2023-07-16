#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
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

//#ifdef __cplusplus
//extern "C" {
//#endif

struct jrtp_session_s
{
    int state;
    char local_address[64];
    u_int16_t local_rtpport;
    u_int16_t local_rtcpport;

    int istcp;  
    int isipv6; 
    int payload;
    int clock;
    int framerate;
    int real_timespec;
    jrtplib::RTPSession jrtp_session;
    pthread_mutex_t mutex;
};
//#ifdef __cplusplus
//};
//#endif

namespace jrtplib
{
class jrtp_session_sched
{
public:
	jrtp_session_sched();
    ~jrtp_session_sched();
    int jrtp_session_sched_add(jrtp_session_t*);
    int jrtp_session_sched_del(jrtp_session_t*);
    int jrtp_session_sched_run();
private:
    std::vector<jrtp_session_t *> _jrtp_session_ver;
    std::vector<jrtplib::SocketType> _jrtp_session_sock;
    pthread_mutex_t mutex;
    int m_stop;
};


jrtp_session_sched::jrtp_session_sched() 
{
    pthread_mutex_init(&mutex,NULL);
}

jrtp_session_sched::~jrtp_session_sched() 
{
    pthread_mutex_destroy(&mutex);
}

int jrtp_session_sched::jrtp_session_sched_add(jrtp_session_t *sec)
{
    jrtplib::SocketType rtpsock = jrtp_session_rtpsock(sec);
    jrtplib::SocketType rtcpsock = jrtp_session_rtcpsock(sec);
    pthread_mutex_lock(&mutex);
    _jrtp_session_sock.push_back(rtpsock);
	_jrtp_session_sock.push_back(rtcpsock);
    _jrtp_session_ver.push_back(sec);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int jrtp_session_sched::jrtp_session_sched_del(jrtp_session_t *sec)
{
    jrtplib::SocketType rtpsock = jrtp_session_rtpsock(sec);
    jrtplib::SocketType rtcpsock = jrtp_session_rtcpsock(sec);
    pthread_mutex_lock(&mutex);
    for (std::vector<jrtplib::SocketType>::iterator it = _jrtp_session_sock.begin();
         it != _jrtp_session_sock.end(); ++it)
    {
        if (*it == rtpsock)
        {
            _jrtp_session_sock.erase(it);
        }
        if (*it == rtcpsock)
        {
            _jrtp_session_sock.erase(it);
        }
    }
    for (std::vector<jrtp_session_t*>::iterator it = _jrtp_session_ver.begin();
         it != _jrtp_session_ver.end(); ++it)
    {
        if (*it == sec)
        {
            _jrtp_session_ver.erase(it);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int jrtp_session_sched::jrtp_session_sched_run()
{
    int8_t i = 0;
	int8_t flags[1024];
	double minInt = 10.0; // wait at most 10 secs
    jrtp_session_t *sec = NULL;
    jrtplib::SocketType m_sockets[1024];
    while(m_stop)
    {
        pthread_mutex_lock(&mutex);
        for (std::vector<jrtplib::SocketType>::iterator it = _jrtp_session_sock.begin();
            it != _jrtp_session_sock.end(); ++it)
        {
            m_sockets[i++] = *it;
        }
        for (std::vector<jrtp_session_t*>::iterator it = _jrtp_session_ver.begin();
            it != _jrtp_session_ver.end(); ++it)
        {
            sec = *it;
            double nextInt = sec->jrtp_session.GetRTCPDelay().GetDouble();

            if (nextInt > 0 && nextInt < minInt)
                minInt = nextInt;
            else if (nextInt <= 0) // call the Poll function to make sure that RTCP packets are sent
            {
                //cout << "RTCP packet should be sent, calling Poll" << endl;
                sec->jrtp_session.Poll();
            }
        }

        RTPTime waitTime(minInt);
        memset(flags, 0, sizeof(flags));
        pthread_mutex_unlock(&mutex);
        int status = RTPSelect(&m_sockets[0], &flags[0], _jrtp_session_sock.size(), waitTime);
        pthread_mutex_lock(&mutex);
        if (status > 0) // some descriptors were set
        {
            for (std::vector<jrtplib::SocketType>::iterator it = _jrtp_session_sock.begin();
                it != _jrtp_session_sock.end(); ++it)
            {
                if (flags[i])
                {
                    int idx = i/2; // two sockets per session
                    if (idx < _jrtp_session_ver.size())
                        _jrtp_session_ver[idx]->jrtp_session.Poll(); 
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}
};

jrtplib::jrtp_session_sched *_jrtp_session_sched = NULL;


#ifdef __cplusplus
extern "C" {
#endif

static int jrtp_session_sched_add(jrtp_session_t *sec)
{
    if(_jrtp_session_sched == NULL)
    {
        _jrtp_session_sched = new jrtplib::jrtp_session_sched();
    }
    else
    {
        return _jrtp_session_sched->jrtp_session_sched_add(sec);
    }
    return -1;
}

static int jrtp_session_sched_del(jrtp_session_t*sec)
{
    if(_jrtp_session_sched != NULL)
    {
         return _jrtp_session_sched->jrtp_session_sched_del(sec);
    }
    return -1;
}
static int jrtp_session_sched_running(void)
{
    if(_jrtp_session_sched != NULL)
    {
         return _jrtp_session_sched->jrtp_session_sched_run();
    }
    return -1;
}

int jrtp_session_event_loop(void *argv)
{
    jrtp_session_sched_running();
    return 0;
}


jrtp_session_t * jrtp_session_alloc(void)
{
    jrtp_session_t *jrtpsess = (jrtp_session_t *)malloc(sizeof(jrtp_session_t));
    if(jrtpsess)
    {
        memset(jrtpsess, 0, sizeof(jrtp_session_t));
        pthread_mutex_init(&jrtpsess->mutex,NULL);
    }
    return jrtpsess;
}

int jrtp_session_destroy(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        jrtpsess->jrtp_session.BYEDestroy(jrtplib::RTPTime(10,0),0,0);
        pthread_mutex_destroy(&jrtpsess->mutex);
        //jrtpsess->jrtp_session.Destroy();
        free(jrtpsess);
    }
    return 0;
}

int jrtp_session_create(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        jrtplib::RTPUDPv4TransmissionParams transparams;
        jrtplib::RTPUDPv6TransmissionParams transparamsv6;
	    jrtplib::RTPSessionParams sessparams;
        jrtplib::RTPTransmitter::TransmissionProtocol proto;
        pthread_mutex_lock(&jrtpsess->mutex);
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
            inet_pton (AF_INET6, jrtpsess->local_address, &addrv6);
            transparamsv6.SetPortbase(jrtpsess->local_rtpport);
            if(strlen((jrtpsess->local_address)))
            {
                //jrtpsess->jrtp_session.rtptrans->SetMulticastInterfaceIP(ntohl(inet_addr(local)));
                transparamsv6.SetBindIP(addrv6);
            }

            if(jrtpsess->jrtp_session.Create(sessparams, &transparamsv6, proto) != 0)
            {
                pthread_mutex_unlock(&jrtpsess->mutex);
                return -1; 
            }  
        }
        else
        {
            transparams.SetPortbase(jrtpsess->local_rtpport);
            if(strlen((jrtpsess->local_address)))
            {
                transparams.SetBindIP(ntohl(inet_addr(jrtpsess->local_address)));
                transparams.SetMulticastInterfaceIP(ntohl(inet_addr(jrtpsess->local_address)));
            }

            if(jrtpsess->jrtp_session.Create(sessparams, &transparams, proto) != 0)
            {
                pthread_mutex_unlock(&jrtpsess->mutex);
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
        pthread_mutex_unlock(&jrtpsess->mutex); 
        return 0;
    }
    return -1;
}

int jrtp_session_stop(jrtp_session_t *jrtpsess)
{
    pthread_mutex_lock(&jrtpsess->mutex);
    jrtpsess->state = 0;
    pthread_mutex_unlock(&jrtpsess->mutex);
    jrtp_session_sched_del(jrtpsess);
    return 0;
}

int jrtp_session_start(jrtp_session_t *jrtpsess)
{
    pthread_mutex_lock(&jrtpsess->mutex);
    jrtpsess->state = 1;
    pthread_mutex_unlock(&jrtpsess->mutex);
    jrtp_session_sched_add(jrtpsess);
    return 0;
}

int jrtp_session_destination_add(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        if(address)
        {
            if(strstr(address, "."))
            {
                jrtpsess->isipv6 = 0;
                jrtplib::RTPIPv4Address destaddr(ntohl(inet_addr(address)), rtpport, rtcpport);
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
            else if(strstr(address, ":"))
            {
                jrtpsess->isipv6 = 1;
                struct in6_addr addrv6;
                inet_pton (AF_INET6, address, &addrv6);
                jrtplib::RTPIPv6Address destaddr(addrv6, rtpport);
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;    
                }
            }
        }
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;
    }
    return -1;
}

int jrtp_session_destination_del(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        if(address)
        {
            if(strstr(address, "."))
            {
                jrtpsess->isipv6 = 0;
                jrtplib::RTPIPv4Address destaddr(ntohl(inet_addr(address)), rtpport, rtcpport);
                if(jrtpsess->jrtp_session.DeleteDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
            else if(strstr(address, ":"))
            {
                jrtpsess->isipv6 = 1;
                struct in6_addr addrv6;
                inet_pton (AF_INET6, address, &addrv6);
                jrtplib::RTPIPv6Address destaddr(addrv6, rtpport);
                if(jrtpsess->jrtp_session.DeleteDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
        }
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;

    }
    return -1;
}

int jrtp_session_multicast_add(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, char *local)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        if(address)
        {
            if(strstr(address, "."))
            {
                jrtpsess->isipv6 = 0;
                jrtplib::RTPIPv4Address destaddr(ntohl(inet_addr(address)), rtpport);
                uint32_t mcastIP = destaddr.GetIP();
                if (!RTPUDPV4TRANS_IS_MCASTADDR(mcastIP) && local)
                {
                    //jrtpsess->jrtp_session.rtptrans->SetMulticastInterfaceIP(ntohl(inet_addr(local)));
                    jrtpsess->jrtp_session.JoinMulticastGroup(destaddr); 
                }
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
            else if(strstr(address, ":"))
            {
                jrtpsess->isipv6 = 1;
                struct in6_addr addrv6;
                inet_pton (AF_INET6, address, &addrv6);
                jrtplib::RTPIPv6Address destaddr(addrv6, rtpport);
       	        in6_addr mcastIP = destaddr.GetIP();
	            if (!RTPUDPV6TRANS_IS_MCASTADDR(mcastIP))
                {
                    //jrtpsess->jrtp_session.SetMulticastInterfaceIndex(0);
                    jrtpsess->jrtp_session.JoinMulticastGroup(destaddr); 
                }
                if(jrtpsess->jrtp_session.AddDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;   
                } 
            }
        }
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;
    }
    return -1;
}

int jrtp_session_multicast_del(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, char *local)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        if(address)
        {
            if(strstr(address, "."))
            {
                jrtpsess->isipv6 = 0;
                jrtplib::RTPIPv4Address destaddr(ntohl(inet_addr(address)), rtpport);
                uint32_t mcastIP = destaddr.GetIP();
                if (!RTPUDPV4TRANS_IS_MCASTADDR(mcastIP))
                {
                    if(local)
                        ;//jrtpsess->jrtp_session.SetMulticastInterfaceIP(ntohl(inet_addr(local)));
                    jrtpsess->jrtp_session.JoinMulticastGroup(destaddr); 
                } 
                if(jrtpsess->jrtp_session.DeleteDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
            else if(strstr(address, ":"))
            {
                jrtpsess->isipv6 = 1;
                struct in6_addr addrv6;
                inet_pton (AF_INET6, address, &addrv6);
                jrtplib::RTPIPv6Address destaddr(addrv6, rtpport);
        	    in6_addr mcastIP = destaddr.GetIP();
	            if (!RTPUDPV6TRANS_IS_MCASTADDR(mcastIP))
                {
                    //jrtpsess->jrtp_session.SetMulticastInterfaceIndex(0);
                    jrtpsess->jrtp_session.LeaveMulticastGroup(destaddr); 
                }
                if(jrtpsess->jrtp_session.DeleteDestination(destaddr) != 0)
                {
                    pthread_mutex_unlock(&jrtpsess->mutex);
                    return -1;
                }
            }
        }
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;
    }
    return -1;
}


int jrtp_session_local_set(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        memset(jrtpsess->local_address, 0, sizeof(jrtpsess->local_address));
        if(address)
            strcpy(jrtpsess->local_address, address);
        jrtpsess->local_rtpport = rtpport;
        jrtpsess->local_rtcpport = rtcpport;
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;
    }
    return -1;
}

int jrtp_session_overtcp_set(jrtp_session_t *jrtpsess, int istcp)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        jrtpsess->istcp = istcp;	
        pthread_mutex_unlock(&jrtpsess->mutex);	
        return 0;
    }
    return -1;
}

int jrtp_session_payload_set(jrtp_session_t *jrtpsess, int pt, int clock)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        jrtpsess->payload = pt;		
	    jrtpsess->clock = clock;
        pthread_mutex_unlock(&jrtpsess->mutex);
        return 0;
    }
    return -1;
}

int jrtp_session_framerate_set(jrtp_session_t *jrtpsess, int framerate)
{
    if(jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
        jrtpsess->framerate = framerate;	
        pthread_mutex_unlock(&jrtpsess->mutex);	
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
        pthread_mutex_lock(&jrtpsess->mutex);
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
        pthread_mutex_unlock(&jrtpsess->mutex);
    }
    return m_sockets;
}

int jrtp_session_rtcpsock(jrtp_session_t *jrtpsess)
{
    jrtplib::SocketType m_sockets = 0;
    jrtplib::RTPTransmissionInfo *pTrans = NULL;
    if (jrtpsess)
    {
        pthread_mutex_lock(&jrtpsess->mutex);
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
        pthread_mutex_unlock(&jrtpsess->mutex);
    }
    return m_sockets;
}

int jrtp_session_rtcpdelay(jrtp_session_t *jrtpsess)
{
    if (jrtpsess)
    {
        int microSeconds = 0;
        pthread_mutex_lock(&jrtpsess->mutex);
        microSeconds = jrtpsess->jrtp_session.GetRTCPDelay().GetMicroSeconds();
        pthread_mutex_unlock(&jrtpsess->mutex);
        return microSeconds;
    }
    return -1;
}

int jrtp_session_rtpdelay(jrtp_session_t *jrtpsess)
{
    if (jrtpsess)
    {
        int real_timespec = 0;
        pthread_mutex_lock(&jrtpsess->mutex);
        real_timespec = jrtplib::RTPTime::CurrentTime().GetMicroSeconds();
        real_timespec = jrtpsess->real_timespec - real_timespec;
        if(jrtpsess->framerate)
            jrtpsess->real_timespec += 1000/jrtpsess->framerate; 
        pthread_mutex_unlock(&jrtpsess->mutex);    
        return real_timespec;
    }
    return -1;
}

int jrtp_session_rtprtcp_sched(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        int status = 0;
        pthread_mutex_lock(&jrtpsess->mutex);
        status = jrtpsess->jrtp_session.Poll();
        pthread_mutex_unlock(&jrtpsess->mutex);
        return status;
    }
    return -1;
}

int jrtp_session_sendto(jrtp_session_t *jrtpsess, const void *data, size_t len,
	                u_int8_t pt, bool mark, u_int32_t timestampinc)
{
    if(jrtpsess)
    {
        int status = 0;
        pthread_mutex_lock(&jrtpsess->mutex);
        if(jrtpsess->state)
        {
            if(pt == 255)
                status = jrtpsess->jrtp_session.SendPacket(data, len, jrtpsess->payload, mark, timestampinc);
            else
                status = jrtpsess->jrtp_session.SendPacket(data, len, pt, mark, timestampinc);
        }
        pthread_mutex_unlock(&jrtpsess->mutex);
        return status;
    }
    return -1;
}

int jrtp_session_recvfrom(jrtp_session_t *jrtpsess)
{
    if(jrtpsess)
    {
        int status = 0;
        pthread_mutex_lock(&jrtpsess->mutex);
        if(!jrtpsess->state)
        {
            pthread_mutex_unlock(&jrtpsess->mutex);
            return status;
        }
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
					//u_int8_t *loaddata = pack->GetPayloadData();
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
        pthread_mutex_unlock(&jrtpsess->mutex);
        return status;
    }
    return -1;
}





#ifdef __cplusplus
};
#endif
