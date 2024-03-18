/*

  This file is a part of JRTPLIB
  Copyright (c) 1999-2017 Jori Liesenborgs

  Contact: jori.liesenborgs@gmail.com

  This library was developed at the Expertise Centre for Digital Media
  (http://www.edm.uhasselt.be), a research center of the Hasselt University
  (http://www.uhasselt.be). The library is based upon work done for 
  my thesis at the School for Knowledge Technology (Belgium/The Netherlands).

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include "rtpconfig.h"

#ifdef RTPDEBUG

#include "rtptypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "rtpdebug.h"

#if 0
#ifdef RTP_SUPPORT_THREAD
#include <jthread/jmutex.h>
//#include <jthread/jmutexautolock.h>
using namespace jthread;
#endif // RTP_SUPPORT_THREAD

struct MemoryInfo
{
	void *ptr;
	size_t size;
	int lineno;
	char *filename;
	
	MemoryInfo *next;
};

#ifdef RTP_SUPPORT_THREAD
JMutex mutex;
#endif // RTP_SUPPORT_THREAD

class MemoryTracker
{
public:
	MemoryTracker() 
	{ 
		firstblock = NULL; 
#ifdef RTP_SUPPORT_THREAD
		mutex.Init();
#endif // RTP_SUPPORT_THREAD
	}
	~MemoryTracker()
	{
#ifdef RTP_SUPPORT_THREAD
		JMutexAutoLock l(mutex);
#endif // RTP_SUPPORT_THREAD

		MemoryInfo *tmp;
		int count = 0;
		
		printf("Checking for memory leaks...\n");fflush(stdout);
		while(firstblock)
		{
			count++;
			printf("Unfreed block %p of %d bytes (file '%s', line %d)\n",firstblock->ptr,(int)firstblock->size,firstblock->filename,firstblock->lineno);;
			
			tmp = firstblock->next;
			
			free(firstblock->ptr);
			if (firstblock->filename)
				free(firstblock->filename);
			free(firstblock);
			firstblock = tmp;
		}
		if (count == 0)
			printf("No memory leaks found\n");
		else
			printf("%d leaks found\n",count);
	}
	
	MemoryInfo *firstblock;	
};

static MemoryTracker memtrack;

void *donew(size_t s,const char *filename,int line)
{	
#ifdef RTP_SUPPORT_THREAD
	JMutexAutoLock l(mutex);
#endif // RTP_SUPPORT_THREAD

	void *p;
	MemoryInfo *meminf;
	
	p = malloc(s);
	meminf = (MemoryInfo *)malloc(sizeof(MemoryInfo));
	
	meminf->ptr = p;
	meminf->size = s;
	meminf->lineno = line;
	meminf->filename = (char *)malloc(strlen(filename)+1);
	strcpy(meminf->filename,filename);
	meminf->next = memtrack.firstblock;
	
	memtrack.firstblock = meminf;
	
	return p;
}

void dodelete(void *p)
{
#ifdef RTP_SUPPORT_THREAD
	JMutexAutoLock l(mutex);
#endif // RTP_SUPPORT_THREAD

	MemoryInfo *tmp,*tmpprev;
	bool found;
	
	tmpprev = NULL;
	tmp = memtrack.firstblock;
	found = false;
	while (tmp != NULL && !found)
	{
		if (tmp->ptr == p)
			found = true;
		else
		{
			tmpprev = tmp;
			tmp = tmp->next;
		}
	}
	if (!found)
	{
		printf("Couldn't free block %p!\n",p);
		fflush(stdout);
	}
	else
	{
		MemoryInfo *n;
		
		fflush(stdout);
		n = tmp->next;
		free(tmp->ptr);
		if (tmp->filename)
			free(tmp->filename);
		free(tmp);
		
		if (tmpprev)
			tmpprev->next = n;
		else
			memtrack.firstblock = n;
	}
}

void *operator new(size_t s)
{
	return donew(s,"UNKNOWN FILE",0);
}

void *operator new[](size_t s)
{
	return donew(s,"UNKNOWN FILE",0);
}

void *operator new(size_t s,char filename[],int line)
{
	return donew(s,filename,line);
}

void *operator new[](size_t s,char filename[],int line)
{
	return donew(s,filename,line);
}

void operator delete(void *p)
{
	dodelete(p);
}

void operator delete[](void *p)
{
	dodelete(p);
}

#endif

/*********************************************/
namespace jrtplib
{

static int (*logcallback)(const char *fmt,...) = nullptr; 
const RtpDebug& RtpDebug::operator << (uint32_t value)const
{
	char buftmp[64];
	memset(buftmp, 0, sizeof(buftmp));
	sprintf(buftmp, "%u", value);
	std::string aabuftmp = std::string(buftmp);
	logstring += aabuftmp;
    return* this;//注意这个返回……
}
const RtpDebug& RtpDebug::operator << (size_t value)const
{
	char buftmp[64];
	memset(buftmp, 0, sizeof(buftmp));
	sprintf(buftmp, "%u", value);
	std::string aabuftmp = std::string(buftmp);
	logstring += aabuftmp;
    return* this;//注意这个返回……
}
const RtpDebug& RtpDebug::operator<<(int value)const
{
	char buftmp[64];
	memset(buftmp, 0, sizeof(buftmp));
	sprintf(buftmp, "%d", value);
	std::string aabuftmp = std::string(buftmp);
	logstring += aabuftmp;
    return* this;//注意这个返回……
}
const RtpDebug& RtpDebug::operator << (char* str)const
{
	std::string aabuftmp = std::string(str);
	logstring += aabuftmp;
    return* this;//注意这个返回……
}
const RtpDebug& RtpDebug::operator << (double val)const
{
	char buftmp[64];
	memset(buftmp, 0, sizeof(buftmp));
	sprintf(buftmp, "%f", val);
	std::string aabuftmp = std::string(buftmp);
	logstring += aabuftmp;
    return* this;//注意这个返回……
}
const RtpDebug & RtpDebug::operator << (std::ostream& (*__pf)(std::ostream&))const
{
      this->logstring += "\r\n";
      if(logcallback != nullptr)
      {
        (logcallback)(this->logstring.c_str());
      }
      this->logstring.clear();
      return* this;
} 

RtpDebug rtpDebug;
/*void RtpDebug::endl(RtpDebug &obj) 
{
    obj.logstring += "\r\n";
	if(logcallback != nullptr)
	{
		(logcallback)(obj.logstring.c_str());
	}
	obj.logstring.clear();
}
*/
/*
const RtpDebug& RtpDebug::endl(RtpDebug *obj)
{
	if(logcallback != nullptr)
	{
		(logcallback)(obj->logstring.c_str());
	}
	obj->logstring.clear();
    return* this;//同样，这里也留意一下……
}
*/

int RtpDebugLogSet(int (*logcb)(const char *fmt,...))
{
	logcallback = logcb;
	return 0;
}

}
#endif // RTPDEBUG

