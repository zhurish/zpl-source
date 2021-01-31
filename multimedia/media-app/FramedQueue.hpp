/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2020 Live Networks, Inc.  All rights reserved.
// Framed Queue
// C++ header

#ifndef _FRAMED_QUEUE_HH
#define _FRAMED_QUEUE_HH

#include <iostream>
#include <mutex>
#include <semaphore.h>
#include <vector>
#include <list>

//#define FRAMED_QUEUE_SYNC

class ringBuffer {
public:
  ringBuffer(const int maxsize=1024*100);
  virtual ~ringBuffer();

  int rngSize();//
  bool rngIsEmpty();
  bool rngIsFull ();
  int 	rngBufGet (unsigned char *buffer, int maxbytes);
  int 	rngBufPut (unsigned char *buffer, int nbytes);
  int 	rngFreeBytes ();
  void 	rngFlush ();

private:
  int pToBuf = 0;		/* offset from start of buffer where to write next */
  int pFromBuf = 0;	/* offset from start of buffer where to read next */
  int bufSize = 0;	/* size of ring in bytes */
  unsigned char *buf = nullptr;		/* pointer to start of buffer */
};


class listData{
public:
  listData(unsigned char *d, int len);
  virtual ~listData();
  unsigned char *data;
  int maxsize;
  int datasize;
};

class vectorBuffer {
public:
  vectorBuffer(int num);
  virtual ~vectorBuffer();

  int vectorBufferDataAdd(unsigned char *d, int l);
  int vectorBufferDataGet(unsigned char *d, int l);
  int vectorBufferDataFlush(void);
  bool vectorBufferDataIsFull();
  bool vectorBufferDataIsEmpty();
private:
  listData * vectorBufferDataCreate(unsigned char *d, int l);
  int vectorBufferDataFree(void);

private:
  std::vector<listData *> data_list;
  std::vector<listData *> data_unused_list;
  int node_num = 0;
};

class listBuffer {
public:
  listBuffer(int num);
  virtual ~listBuffer();

  int listBufferDataAdd(unsigned char *d, int l);
  int listBufferDataGet(unsigned char *d, int l);
  int listBufferDataFlush(void);
  bool listBufferDataIsFull();
  bool listBufferDataIsEmpty();
private:
  listData * listBufferDataCreate(unsigned char *d, int l);
  int listBufferDataFree(void);

private:
  std::list<listData *> data_list;
  std::list<listData *> data_unused_list;
  int node_num = 0;
};

//template <typename DT>

class FramedQueue {
public:
  /**/
  static const int FRAMED_QUEUE_RING = 0;
  static const int FRAMED_QUEUE_LIST = 1;
  static const int FRAMED_QUEUE_MAP = 2;

  FramedQueue(int type);
  //FramedQueue(int type, DT *p);
  virtual ~FramedQueue();

  int FramedQueueInit(int size, bool syncsize);
  int FramedQueueDataPut(unsigned char *d, int l);
  int FramedQueueDataGet(unsigned char *d, int l);
  int FramedQueueDataSizeGet();

  int FramedQueueDataFlush(void);
  bool FramedQueueDataIsFull();
  bool FramedQueueDataIsEmpty();
  int FramedQueueDataFd();

  void FramedQueueWait();
  void FramedQueuePost();  
  int FramedQueueDataReady();

  static void FramedQueueEventTriggerHandler(void* clientData);
  bool 	FramedQueueIsStart ();


private:
  //DT *device = nullptr;
#ifdef FRAMED_QUEUE_SYNC
  sem_t m_sem;
#endif  
  std::mutex m_mutex;
  listBuffer *list_buffer = nullptr;
  vectorBuffer *vector_buffer = nullptr;
  ringBuffer *ring_buffer = nullptr;
  int m_type = 0;
  bool event = false;
  int m_fd[2] = {0, 0}; 
};

#endif
