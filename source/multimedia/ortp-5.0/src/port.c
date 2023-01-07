/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of oRTP 
 * (see https://gitlab.linphone.org/BC/public/ortp).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef HAVE_CONFIG_H
#include "ortp-config.h"
#endif
#include "ortp/logging.h"
#include "ortp/port.h"
#include "ortp/str_utils.h"
#include "utils.h"
//#include <ortp/bctport.h>
//#include <bctoolbox/charconv.h>

#if	defined(_WIN32) && !defined(_WIN32_WCE)
#include <process.h>
#endif

#ifdef HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif

static void *ortp_libc_malloc(size_t sz){
    void *ptr = malloc(sz);
    if(ptr != NULL)
        memset(ptr, 0, sz);
    return ptr;
}

static void *ortp_libc_realloc(void *ptr, size_t sz){
	return realloc(ptr,sz);
}

static void ortp_libc_free(void*ptr){
	free(ptr);
}

static bool_t allocator_used=FALSE;

static OrtpMemoryFunctions ortp_allocator={
	ortp_libc_malloc,
	ortp_libc_realloc,
	ortp_libc_free
};

void ortp_set_memory_functions(OrtpMemoryFunctions *functions){
	if (allocator_used){
		ortp_fatal("ortp_set_memory_functions() must be called before "
		"first use of ortp_malloc or ortp_realloc");
		return;
	}
	ortp_allocator=*functions;
}

void* ortp_malloc(size_t sz){
	allocator_used=TRUE;
	return ortp_allocator.malloc_fun(sz);
}

void* ortp_realloc(void *ptr, size_t sz){
	allocator_used=TRUE;
	return ortp_allocator.realloc_fun(ptr,sz);
}

void ortp_free(void* ptr){
	ortp_allocator.free_fun(ptr);
}

void * ortp_malloc0(size_t size){
	void *ptr=ortp_malloc(size);
	memset(ptr,0,size);
	return ptr;
}

char * ortp_strdup(const char *tmp){
	size_t sz;
	char *ret;
	if (tmp==NULL)
	  return NULL;
	sz=strlen(tmp)+1;
	ret=(char*)ortp_malloc(sz);
	strcpy(ret,tmp);
	ret[sz-1]='\0';
	return ret;
}

/*
 * this method is an utility method that calls fnctl() on UNIX or
 * ioctlsocket on Win32.
 * int retrun the result of the system method
 */
int set_non_blocking_socket (ortp_socket_t sock){
#if	!defined(_WIN32) && !defined(_WIN32_WCE)
	return fcntl (sock, F_SETFL, fcntl(sock,F_GETFL) | O_NONBLOCK);
#else
	unsigned long nonBlock = 1;
	return ioctlsocket(sock, FIONBIO , &nonBlock);
#endif
}

/*
 * this method is an utility method that calls fnctl() on UNIX or
 * ioctlsocket on Win32.
 * int retrun the result of the system method
 */
int set_blocking_socket (ortp_socket_t sock){
#if	!defined(_WIN32) && !defined(_WIN32_WCE)
	return fcntl (sock, F_SETFL, fcntl(sock, F_GETFL) & ~O_NONBLOCK);
#else
	unsigned long nonBlock = 0;
	return ioctlsocket(sock, FIONBIO , &nonBlock);
#endif
}


/*
 * this method is an utility method that calls close() on UNIX or
 * closesocket on Win32.
 * int retrun the result of the system method
 */
int close_socket(ortp_socket_t sock){
#if	!defined(_WIN32) && !defined(_WIN32_WCE)
	return close (sock);
#else
	return closesocket(sock);
#endif
}

#if	!defined(_WIN32) && !defined(_WIN32_WCE)
	/* Use UNIX inet_aton method */
#else
	int inet_aton (const char * cp, struct in_addr * addr)
	{
		int retval;

		retval = inet_pton (AF_INET, cp, addr);

		return retval == 1 ? 1 : 0;
	}
#endif

char *ortp_strndup(const char *str,int n){
	int min=MIN((int)strlen(str),n)+1;
	char *ret=(char*)ortp_malloc(min);
	strncpy(ret,str,min);
	ret[min-1]='\0';
	return ret;
}

#if	!defined(_WIN32) && !defined(_WIN32_WCE)
int __ortp_thread_join(ortp_thread_t thread, void **ptr){
	int err=pthread_join(thread,ptr);
	if (err!=0) {
		ortp_error("pthread_join error: %s",strerror(err));
	}
	return err;
}

int __ortp_thread_create(ortp_thread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg){
	pthread_attr_t my_attr;
	pthread_attr_init(&my_attr);
	if (attr)
		my_attr = *attr;
#ifdef ORTP_DEFAULT_THREAD_STACK_SIZE
	if (ORTP_DEFAULT_THREAD_STACK_SIZE!=0)
		pthread_attr_setstacksize(&my_attr, ORTP_DEFAULT_THREAD_STACK_SIZE);
#endif
	return pthread_create(thread, &my_attr, routine, arg);
}

unsigned long __ortp_thread_self(void) {
	return (unsigned long)pthread_self();
}

#endif
#if	defined(_WIN32) || defined(_WIN32_WCE)

int WIN_mutex_init(ortp_mutex_t *mutex, void *attr)
{
#ifdef ORTP_WINDOWS_DESKTOP
	*mutex=CreateMutex(NULL, FALSE, NULL);
#else
	InitializeSRWLock(mutex);
#endif
	return 0;
}

int WIN_mutex_lock(ortp_mutex_t * hMutex)
{
#ifdef ORTP_WINDOWS_DESKTOP
	WaitForSingleObject(*hMutex, INFINITE); /* == WAIT_TIMEOUT; */
#else
	AcquireSRWLockExclusive(hMutex);
#endif
	return 0;
}

int WIN_mutex_unlock(ortp_mutex_t * hMutex)
{
#ifdef ORTP_WINDOWS_DESKTOP
	ReleaseMutex(*hMutex);
#else
	ReleaseSRWLockExclusive(hMutex);
#endif
	return 0;
}

int WIN_mutex_destroy(ortp_mutex_t * hMutex)
{
#ifdef ORTP_WINDOWS_DESKTOP
	CloseHandle(*hMutex);
#endif
	return 0;
}

typedef struct thread_param{
	void * (*func)(void *);
	void * arg;
}thread_param_t;

static unsigned WINAPI thread_starter(void *data){
	thread_param_t *params=(thread_param_t*)data;
	params->func(params->arg);
	ortp_free(data);
	return 0;
}

#if defined _WIN32_WCE
#    define _beginthreadex	CreateThread
#    define	_endthreadex	ExitThread
#endif

int WIN_thread_create(ortp_thread_t *th, void *attr, void * (*func)(void *), void *data)
{
	thread_param_t *params=ortp_new(thread_param_t,1);
	params->func=func;
	params->arg=data;
	*th=(HANDLE)_beginthreadex( NULL, 0, thread_starter, params, 0, NULL);
	return 0;
}

int WIN_thread_join(ortp_thread_t thread_h, void **unused)
{
	if (thread_h!=NULL)
	{
		WaitForSingleObjectEx(thread_h, INFINITE, FALSE);
		CloseHandle(thread_h);
	}
	return 0;
}

unsigned long WIN_thread_self(void) {
	return (unsigned long)GetCurrentThreadId();
}

int WIN_cond_init(ortp_cond_t *cond, void *attr)
{
#ifdef ORTP_WINDOWS_DESKTOP
	*cond=CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	InitializeConditionVariable(cond);
#endif
	return 0;
}

int WIN_cond_wait(ortp_cond_t* hCond, ortp_mutex_t * hMutex)
{
#ifdef ORTP_WINDOWS_DESKTOP
	//gulp: this is not very atomic ! bug here ?
	WIN_mutex_unlock(hMutex);
	WaitForSingleObject(*hCond, INFINITE);
	WIN_mutex_lock(hMutex);
#else
	SleepConditionVariableSRW(hCond, hMutex, INFINITE, 0);
#endif
	return 0;
}

int WIN_cond_signal(ortp_cond_t * hCond)
{
#ifdef ORTP_WINDOWS_DESKTOP
	SetEvent(*hCond);
#else
	WakeConditionVariable(hCond);
#endif
	return 0;
}

int WIN_cond_broadcast(ortp_cond_t * hCond)
{
	WIN_cond_signal(hCond);
	return 0;
}

int WIN_cond_destroy(ortp_cond_t * hCond)
{
#ifdef ORTP_WINDOWS_DESKTOP
	CloseHandle(*hCond);
#endif
	return 0;
}

#if defined(_WIN32_WCE)
#include <time.h>

const char * ortp_strerror(DWORD value) {
	static TCHAR msgBuf[256];
	FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			value,
			0, // Default language
			(LPTSTR) &msgBuf,
			0,
			NULL
	);
	return (const char *)msgBuf;
}

int
gettimeofday (struct timeval *tv, void *tz)
{
  DWORD timemillis = GetTickCount();
  tv->tv_sec  = timemillis/1000;
  tv->tv_usec = (timemillis - (tv->tv_sec*1000)) * 1000;
  return 0;
}

#else

int ortp_gettimeofday (struct timeval *tv, void* tz)
{
	union
	{
		__int64 ns100; /*time since 1 Jan 1601 in 100ns units */
		FILETIME fileTime;
	} now;

	GetSystemTimeAsFileTime (&now.fileTime);
	tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
	return (0);
}

#endif

const char *getWinSocketError(int error)
{
	static char buf[80];

	switch (error)
	{
		case WSANOTINITIALISED: return "Windows sockets not initialized : call WSAStartup";
		case WSAEADDRINUSE:		return "Local Address already in use";
		case WSAEADDRNOTAVAIL:	return "The specified address is not a valid address for this machine";
		case WSAEINVAL:			return "The socket is already bound to an address.";
		case WSAENOBUFS:		return "Not enough buffers available, too many connections.";
		case WSAENOTSOCK:		return "The descriptor is not a socket.";
		case WSAECONNRESET:		return "Connection reset by peer";

		default :
			sprintf(buf, "Error code : %d", error);
			return buf;
		break;
	}

	return buf;
}

#ifdef _WORKAROUND_MINGW32_BUGS
char * WSAAPI gai_strerror(int errnum){
	 return (char*)getWinSocketError(errnum);
}
#endif

#endif

#ifndef _WIN32

#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/stat.h>

/* portable named pipes */

#ifdef HAVE_SYS_SHM_H

#endif

#elif defined(_WIN32) && !defined(_WIN32_WCE)

#endif


#ifdef __MACH__
#include <sys/types.h>
#include <sys/timeb.h>
#endif

void _ortp_get_cur_time(ortpTimeSpec *ret, bool_t realtime){
#if defined(_WIN32_WCE) || defined(_WIN32)
#if defined( ORTP_WINDOWS_DESKTOP ) && !defined(ENABLE_MICROSOFT_STORE_APP) && !defined(ORTP_WINDOWS_UWP)
	DWORD timemillis;
#	if defined(_WIN32_WCE)
	timemillis=GetTickCount();
#	else
	timemillis=timeGetTime();
#	endif
	ret->tv_sec=timemillis/1000;
	ret->tv_nsec=(timemillis%1000)*1000000LL;
#else
	ULONGLONG timemillis = GetTickCount64();
	ret->tv_sec = timemillis / 1000;
	ret->tv_nsec = (timemillis % 1000) * 1000000LL;
#endif
#elif defined(__MACH__) && defined(__GNUC__) && (__GNUC__ >= 3)
	struct timeval tv;
	gettimeofday(&tv, NULL);
	ret->tv_sec=tv.tv_sec;
	ret->tv_nsec=tv.tv_usec*1000LL;
#elif defined(__MACH__)
	struct timeb time_val;

	ftime (&time_val);
	ret->tv_sec = time_val.time;
	ret->tv_nsec = time_val.millitm * 1000000LL;
#else
	struct timespec ts;
	if (clock_gettime(realtime ? CLOCK_REALTIME : CLOCK_MONOTONIC,&ts)<0){
		ortp_fatal("clock_gettime() doesn't work: %s",strerror(errno));
	}
	ret->tv_sec=ts.tv_sec;
	ret->tv_nsec=ts.tv_nsec;
#endif
}

void ortp_get_cur_time(ortpTimeSpec *ret){
	_ortp_get_cur_time(ret, FALSE);
}


uint64_t ortp_get_cur_time_ms(void) {
	ortpTimeSpec ts;
	ortp_get_cur_time(&ts);
	return (ts.tv_sec * 1000LL) + ((ts.tv_nsec + 500000LL) / 1000000LL);
}

void ortp_sleep_ms(int ms){
#ifdef _WIN32
#ifdef ORTP_WINDOWS_DESKTOP
	Sleep(ms);
#else
	HANDLE sleepEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
	if (!sleepEvent) return;
	WaitForSingleObjectEx(sleepEvent, ms, FALSE);
	CloseHandle(sleepEvent);
#endif
#else
	struct timespec ts;
	ts.tv_sec=ms/1000;
	ts.tv_nsec=(ms%1000)*1000000LL;
	nanosleep(&ts,NULL);
#endif
}

void ortp_sleep_until(const ortpTimeSpec *ts){
#ifdef __linux__
	struct timespec rq;
	rq.tv_sec=ts->tv_sec;
	rq.tv_nsec=ts->tv_nsec;
	while (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &rq, NULL)==-1 && errno==EINTR){
	}
#else
	ortpTimeSpec current;
	ortpTimeSpec diff;
	_ortp_get_cur_time(&current, TRUE);
	diff.tv_sec=ts->tv_sec-current.tv_sec;
	diff.tv_nsec=ts->tv_nsec-current.tv_nsec;
	if (diff.tv_nsec<0){
		diff.tv_nsec+=1000000000LL;
		diff.tv_sec-=1;
	}
#ifdef _WIN32
		ortp_sleep_ms((int)((diff.tv_sec * 1000LL) + (diff.tv_nsec/1000000LL)));
#else
	{
		struct timespec dur,rem;
		dur.tv_sec=diff.tv_sec;
		dur.tv_nsec=diff.tv_nsec;
		while (nanosleep(&dur,&rem)==-1 && errno==EINTR){
			dur=rem;
		};
	}
#endif
#endif
}

#if defined(_WIN32) && !defined(_MSC_VER)
char* strtok_r(char *str, const char *delim, char **nextp){
	char *ret;

	if (str == NULL){
		str = *nextp;
	}
	str += strspn(str, delim);
	if (*str == '\0'){
		return NULL;
	}
	ret = str;
	str += strcspn(str, delim);
	if (*str){
		*str++ = '\0';
	}
	*nextp = str;
	return ret;
}
#endif


#if defined(_WIN32)

#if !defined(_MSC_VER)
#include <wincrypt.h>
static int ortp_wincrypto_random(unsigned int *rand_number){
	static HCRYPTPROV hProv=(HCRYPTPROV)-1;
	static int initd=0;

	if (!initd){
		if (!CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
			ortp_error("ortp_wincrypto_random(): Could not acquire a windows crypto context");
			return -1;
		}
		initd=TRUE;
	}
	if (hProv==(HCRYPTPROV)-1)
		return -1;

	if (!CryptGenRandom(hProv,4,(BYTE*)rand_number)){
		ortp_error("ortp_wincrypto_random(): CryptGenRandom() failed.");
		return -1;
	}
	return 0;
}
#endif
#elif defined(__QNXNTO__) || ((defined(__linux__) || defined(__APPLE__)) && !defined(HAVE_ARC4RANDOM))

static unsigned int ortp_urandom(void) {
	static int fd=-1;
	if (fd==-1) fd=open("/dev/urandom",O_RDONLY);
	if (fd!=-1){
		unsigned int tmp;
		if (read(fd,&tmp,4)!=4){
			ortp_error("Reading /dev/urandom failed.");
		}else return tmp;
	} else ortp_error("Could not open /dev/urandom");
	return (unsigned int) random();
}

#endif




unsigned int ortp_random(void){
#ifdef HAVE_ARC4RANDOM
#if defined(__QNXNTO__) // There is a false positive with blackberry build
	return ortp_urandom();
#else
	return arc4random();
#endif
#elif defined(__linux__) || defined(__APPLE__)
	return ortp_urandom();
#elif defined(_WIN32)
	static int initd=0;
	unsigned int ret;
#ifdef _MSC_VER
	/*rand_s() is pretty nice and simple function but is not wrapped by mingw.*/

	if (rand_s(&ret)==0){
		return ret;
	}
#else
	if (ortp_wincrypto_random(&ret)==0){
		return ret;
	}
#endif
	/* Windows's rand() is unsecure but is used as a fallback*/
	if (!initd) {
		struct timeval tv;
		ortp_gettimeofday(&tv,NULL);
		srand((unsigned int)tv.tv_sec+tv.tv_usec);
		initd=1;
		ortp_warning("ortp: Random generator is using rand(), this is unsecure !");
	}
	return rand()<<16 | rand();
#endif
}

bool_t ortp_is_multicast_addr(const struct ipstack_sockaddr *addr) {

	switch (addr->sa_family) {
		case IPSTACK_AF_INET:
			return IPSTACK_IN_MULTICAST(ntohl(((struct ipstack_sockaddr_in *) addr)->sin_addr.s_addr));
		case IPSTACK_AF_INET6:
			return IPSTACK_IN6_IS_ADDR_MULTICAST(&(((struct ipstack_sockaddr_in6 *) addr)->sin6_addr));
		default:
			return FALSE;
	}

}


int ortp_sockaddr_to_address(const struct ipstack_sockaddr *sa, socklen_t salen, char *ip, size_t ip_size, int *port) {
		inet_ntop(sa->sa_family, &((struct ipstack_sockaddr_in*)sa)->sin_addr, ip, ip_size);
        if(sa->sa_family == AF_INET && port)
			*port = ntohs(((struct ipstack_sockaddr_in *)sa)->sin_port);
        if(sa->sa_family == AF_INET6 && port)
			*port = ntohs(((struct ipstack_sockaddr_in6 *)sa)->sin6_port);
		return 0;
}

int ortp_sockaddr_to_print_address(struct ipstack_sockaddr *sa, socklen_t salen, char *printable_ip, size_t printable_ip_size) {
        if ((sa->sa_family == 0) || (salen == 0)) {
                snprintf(printable_ip, printable_ip_size, "no-addr");
                return 0;
        } else {
            char ip[64];
            int port;
			ortp_sockaddr_to_address(sa, salen, ip, sizeof(ip), &port);
			if (sa->sa_family == AF_INET)
			    snprintf(printable_ip, printable_ip_size, "%s:%d", ip, port);
            else if (sa->sa_family == AF_INET6)
                    snprintf(printable_ip, printable_ip_size, "[%s]:%d", ip, port);
            return 0;
        }
}

int ortp_address_to_sockaddr(int sin_family, char *ip, int port,
                             const struct ipstack_sockaddr *sa, socklen_t *salen) {
    struct ipstack_sockaddr_in *addr = (struct ipstack_sockaddr_in *)sa;
    struct ipstack_sockaddr_in6 *addr6 = (struct ipstack_sockaddr_in6 *)sa;
    if(sin_family == AF_INET)
    {
        memset(addr, 0, sizeof(struct ipstack_sockaddr_in));
        addr->sin_family = AF_INET;
        /* If the modbus port is < to 1024, we need the setuid root. */
        addr->sin_port = htons(port);
        if (ip == NULL) {
            addr->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            addr->sin_addr.s_addr = inet_addr(ip);
        }
        if(salen)
            *salen = sizeof(struct ipstack_sockaddr_in);
    }
    if(sin_family == AF_INET6)
    {
        memset(addr6, 0, sizeof(struct ipstack_sockaddr_in6));
        addr6->sin6_family = AF_INET6;
        /* If the modbus port is < to 1024, we need the setuid root. */
        addr6->sin6_port = htons(port);
        if (ip == NULL) {
            memset(&addr6->sin6_addr.s6_addr, 0, sizeof(addr6->sin6_addr.s6_addr));
        } else {
            //inet6_aton();
            //addr6->sin6_addr.s6_addr = 0;
        }
        if(salen)
            *salen = sizeof(struct ipstack_sockaddr_in6);
    }
    return 0;
}

bool_t ortp_sockaddr_equals(const struct ipstack_sockaddr * sa, const struct ipstack_sockaddr * sb) {

        if (sa->sa_family != sb->sa_family)
                return FALSE;

        if (sa->sa_family == AF_INET) {
                if ((((struct ipstack_sockaddr_in*)sa)->sin_addr.s_addr != ((struct ipstack_sockaddr_in*)sb)->sin_addr.s_addr
                         || ((struct ipstack_sockaddr_in*)sa)->sin_port != ((struct ipstack_sockaddr_in*)sb)->sin_port))
                        return FALSE;
        } else if (sa->sa_family == AF_INET6) {
                if (memcmp(&((struct ipstack_sockaddr_in6*)sa)->sin6_addr
                                   , &((struct ipstack_sockaddr_in6*)sb)->sin6_addr
                                   , sizeof(struct in6_addr)) !=0
                        || ((struct ipstack_sockaddr_in6*)sa)->sin6_port != ((struct ipstack_sockaddr_in6*)sb)->sin6_port)
                        return FALSE;
        } else {
                ortp_warning ("Cannot compare family type [%d]", sa->sa_family);
                return FALSE;
        }
        return TRUE;

}
