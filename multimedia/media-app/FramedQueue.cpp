/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.FAST

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2020 Live Networks, Inc.  All rights reserved.
// Framed Queue
// Implementation
#include <sstream>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include <mutex>
#include <list>
#include <vector>
#include <semaphore.h>
#include <liveMedia/liveMedia.hh>
#include <groupsock/GroupsockHelper.hh>
#include "FramedQueue.hpp"

////////// FramedFileSource //////////
#define min(a, b) ((a) > (b)) ? (b) : (a)

ringBuffer::ringBuffer(const int maxsize)
{
  zpl_uint8 *buffer;
  buffer = new zpl_uint8 [maxsize + 1];
  this->bufSize = maxsize;
  this->pToBuf = 0;
  this->pFromBuf = 0;
  this->buf = buffer;
}
ringBuffer::~ringBuffer()
{
  delete[] this->buf;
}

void ringBuffer::rngFlush()
{
  this->pToBuf = 0;
  this->pFromBuf = 0;
}

int ringBuffer::rngBufGet(zpl_uint8 *buffer, int maxbytes)
{
  int bytesgot = 0;
  int toBuf = this->pToBuf;
  int bytes2;
  int pRngTmp = 0;

  if (toBuf >= this->pFromBuf)
  {
    /* pToBuf has not wrapped around */
    bytesgot = min(maxbytes, toBuf - this->pFromBuf);
    memcpy(buffer, &this->buf[this->pFromBuf], bytesgot);
    this->pFromBuf += bytesgot;
  }
  else
  {
    /* pToBuf has wrapped around.  Grab chars up to the end of the
    * buffer, then wrap around if we need to. */

    bytesgot = min(maxbytes, this->bufSize - this->pFromBuf);
    memcpy(buffer, &this->buf[this->pFromBuf], bytesgot);
    pRngTmp = this->pFromBuf + bytesgot;

    /* If pFromBuf is equal to bufSize, we've read the entire buffer,
    * and need to wrap now.  If bytesgot < maxbytes, copy some more chars
    * in now. */

    if (pRngTmp == this->bufSize)
    {
      bytes2 = min(maxbytes - bytesgot, toBuf);
      memcpy(buffer + bytesgot, this->buf, bytes2);
      this->pFromBuf = bytes2;
      bytesgot += bytes2;
    }
    else
      this->pFromBuf = pRngTmp;
  }
  return (bytesgot);
}
/*******************************************************************************
*
* rngBufPut - put bytes into a ring buffer
*
* This routine puts bytes from <buffer> into ring buffer <ringId>.  The
* specified number of bytes will be put into the ring, up to the number of
* bytes available in the ring.
*
* INTERNAL
* Always leaves at least one byte empty between pToBuf and pFromBuf, to
* eliminate ambiguities which could otherwise occur when the two pointers
* are equal.
*
* RETURNS:
* The number of bytes actually put into the ring buffer;
* it may be less than number requested, even zero,
* if there is insufficient room in the ring buffer at the time of the call.
*/

int ringBuffer::rngBufPut(zpl_uint8 *buffer, /* buffer to get data from       */
                          zpl_uint32 nbytes    /* number of bytes to try to put */
)
{
  int bytesput = 0;
  int fromBuf = this->pFromBuf;
  int bytes2;
  int pRngTmp = 0;

  if (fromBuf > this->pToBuf)
  {
    /* fromBuf is ahead of pToBuf.  We can fill up to two bytes
	 * before it */

    bytesput = min(nbytes, fromBuf - this->pToBuf - 1);
    memcpy(&this->buf[this->pToBuf], buffer, bytesput);
    this->pToBuf += bytesput;
  }
  else if (fromBuf == 0)
  {
    /* pFromBuf is at the beginning of the buffer.  We can fill till
	 * the next-to-last element */

    bytesput = min(nbytes, this->bufSize - this->pToBuf - 1);
    memcpy(&this->buf[this->pToBuf], buffer, bytesput);
    this->pToBuf += bytesput;
  }
  else
  {
    /* pFromBuf has wrapped around, and its not 0, so we can fill
	 * at least to the end of the ring buffer.  Do so, then see if
	 * we need to wrap and put more at the beginning of the buffer. */

    bytesput = min(nbytes, this->bufSize - this->pToBuf);
    memcpy(&this->buf[this->pToBuf], buffer, bytesput);
    pRngTmp = this->pToBuf + bytesput;

    if (pRngTmp == this->bufSize)
    {
      /* We need to wrap, and perhaps put some more chars */

      bytes2 = min(nbytes - bytesput, fromBuf - 1);
      memcpy(this->buf, buffer + bytesput, bytes2);
      this->pToBuf = bytes2;
      bytesput += bytes2;
    }
    else
      this->pToBuf = pRngTmp;
  }
  return (bytesput);
}
/*******************************************************************************
*
* rngIsEmpty - test if a ring buffer is empty
*
* This routine determines if a specified ring buffer is empty.
*
* RETURNS:
* zpl_true if empty, zpl_false if not.
*/

bool ringBuffer::rngIsEmpty()
{
  return (this->pToBuf == this->pFromBuf) ? true : false;
}
/*******************************************************************************
*
* rngIsFull - test if a ring buffer is full (no more room)
*
* This routine determines if a specified ring buffer is completely full.
*
* RETURNS:
* zpl_true if full, zpl_false if not.
*/

bool ringBuffer::rngIsFull()
{
  int n = this->pToBuf - this->pFromBuf + 1;
  return ((n == 0) || (n == this->bufSize)) ? true : false;
}
/*******************************************************************************
*
* rngFreeBytes - determine the number of free bytes in a ring buffer
*
* This routine determines the number of bytes currently unused in a specified
* ring buffer.
*
* RETURNS: The number of unused bytes in the ring buffer.
*/

int ringBuffer::rngFreeBytes()
{
  int n = this->pFromBuf - this->pToBuf - 1;

  if (n < 0)
    n += this->bufSize;
  return (n);
}

//////////////////////listData///////////////////////
listData::listData(zpl_uint8 *d, int len)
{
  this->maxsize = ((len + 3) / 4) * 4;
  this->datasize = len;
  this->data = new zpl_uint8 [this->maxsize];
  memcpy(this->data, d, len);
}

listData::~listData()
{
  if (this->data)
    delete[] this->data;
}

//////////////////////vectorBuffer///////////////////////
listData *vectorBuffer::vectorBufferDataCreate(zpl_uint8 *d, int l)
{
  listData *t = nullptr;
  if (data_unused_list.size())
  {
    std::vector<listData *>::iterator it;
    for (it = data_unused_list.begin(); it != data_unused_list.end(); ++it)
    {
      if (*it != nullptr)
      {
        t = *it;
        if (t->maxsize >= l)
          break;
      }
    }
    if (t != nullptr)
    {
      data_unused_list.erase(it);
      t->datasize = l;
      memcpy(t->data, d, l);
      return t;
    }
    return nullptr;
  }
  else
  {
    t = new listData(d, l);
    return t;
  }
  return nullptr;
}

int vectorBuffer::vectorBufferDataFlush()
{
  for (std::vector<listData *>::iterator it = data_list.begin(); it != data_list.end(); ++it)
  {
    if (*it != nullptr)
    {
      data_list.erase(it);
      data_unused_list.push_back(*it);
    }
  }
  return 0;
}

bool vectorBuffer::vectorBufferDataIsFull()
{
  if (data_list.size() >= node_num)
  {
    return true;
  }
  return false;
}
bool vectorBuffer::vectorBufferDataIsEmpty()
{
  if (data_list.size() == 0)
  {
    return true;
  }
  return false;
}
int vectorBuffer::vectorBufferDataAdd(zpl_uint8 *d, int l)
{
  if ((data_list.size() + data_unused_list.size()) >= node_num)
  {
    return -1;
  }

  listData *t = vectorBufferDataCreate(d, l);
  if (t != nullptr)
  {
    data_list.push_back(t);
    return 0;
  }
  return -1;
}

int vectorBuffer::vectorBufferDataFree(void)
{
  for (std::vector<listData *>::iterator it = data_list.begin(); it != data_list.end(); ++it)
  {
    if (*it != nullptr)
    {
      data_list.erase(it);
      delete *it;
    }
  }
  for (std::vector<listData *>::iterator uit = data_unused_list.begin(); uit != data_unused_list.end(); ++uit)
  {
    if (*uit != nullptr)
    {
      data_unused_list.erase(uit);
      delete *uit;
    }
  }
  return 0;
}

vectorBuffer::vectorBuffer(int num)
{
  data_list.clear();
  data_unused_list.clear();
  node_num = num;
}

vectorBuffer::~vectorBuffer()
{
  vectorBufferDataFree();
  data_list.clear();
  data_unused_list.clear();
}

int vectorBuffer::vectorBufferDataGet(zpl_uint8 *d, int l)
{
  if (data_list.size())
  {
    listData *i = nullptr; // = data_list.at(0);
    std::vector<listData *>::iterator it = data_list.begin();
    i = *it;
    if (i != nullptr && i->data != nullptr)
    {
      data_list.erase(it);
      data_unused_list.push_back(*it);
      memcpy(d, i->data, min(l, i->datasize));
      return min(l, i->datasize);
    }
    return 0;
  }
  return 0;
}

//////////////////////listBuffer///////////////////////
listData *listBuffer::listBufferDataCreate(zpl_uint8 *d, int l)
{
  listData *t = nullptr;
  if (data_unused_list.size())
  {
    std::list<listData *>::iterator it;
    for (it = data_unused_list.begin(); it != data_unused_list.end(); ++it)
    {
      if (*it != nullptr)
      {
        t = *it;
        if (t->maxsize >= l)
          break;
      }
    }
    if (t != nullptr)
    {
      data_unused_list.erase(it);
      t->datasize = l;
      memcpy(t->data, d, l);
      return t;
    }
    return nullptr;
  }
  else
  {
    t = new listData(d, l);
    return t;
  }
  return nullptr;
}

int listBuffer::listBufferDataFlush()
{
  for (std::list<listData *>::iterator it = data_list.begin(); it != data_list.end(); ++it)
  {
    if (*it != nullptr)
    {
      data_list.erase(it);
      data_unused_list.push_back(*it);
    }
  }
  return 0;
}

bool listBuffer::listBufferDataIsFull()
{
  if (data_list.size() >= node_num)
  {
    return true;
  }
  return false;
}
bool listBuffer::listBufferDataIsEmpty()
{
  if (data_list.size() == 0)
  {
    return true;
  }
  return false;
}
int listBuffer::listBufferDataAdd(zpl_uint8 *d, int l)
{
  if ((data_list.size() + data_unused_list.size()) >= node_num)
  {
    return -1;
  }

  listData *t = listBufferDataCreate(d, l);
  if (t != nullptr)
  {
    data_list.push_back(t);
    return 0;
  }
  return -1;
}

int listBuffer::listBufferDataFree(void)
{
  for (std::list<listData *>::iterator it = data_list.begin(); it != data_list.end(); ++it)
  {
    if (*it != nullptr)
    {
      data_list.erase(it);
      delete *it;
    }
  }
  for (std::list<listData *>::iterator uit = data_unused_list.begin(); uit != data_unused_list.end(); ++uit)
  {
    if (*uit != nullptr)
    {
      data_unused_list.erase(uit);
      delete *uit;
    }
  }
  return 0;
}

listBuffer::listBuffer(int num)
{
  data_list.clear();
  data_unused_list.clear();
  node_num = num;
}

listBuffer::~listBuffer()
{
  listBufferDataFree();
  data_list.clear();
  data_unused_list.clear();
}

int listBuffer::listBufferDataGet(zpl_uint8 *d, int l)
{
  if (data_list.size())
  {
    listData *i = nullptr; // = data_list.at(0);
    std::list<listData *>::iterator it = data_list.begin();
    i = *it;
    if (i != nullptr && i->data != nullptr)
    {
      data_list.erase(it);
      data_unused_list.push_back(*it);
      memcpy(d, i->data, min(l, i->datasize));
      return min(l, i->datasize);
    }
    return 0;
  }
  return 0;
}

//////////////////////FramedQueue///////////////////////

FramedQueue::FramedQueue(zpl_uint32 type)
{
  m_type = type;
  m_fd[0] = m_fd[1] = 0;
  event = false;
  list_buffer = nullptr;
  vector_buffer = nullptr;
  ring_buffer = nullptr;
}
/*
FramedQueue::FramedQueue(zpl_uint32 type, DT *p)
{
  m_type = type;
  m_fd[0] = m_fd[1] = 0;
  device = p;
}
*/

FramedQueue::~FramedQueue()
{
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer != nullptr)
      delete vector_buffer;
  }
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer != nullptr)
      delete ring_buffer;
  }
  else if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer != nullptr)
      delete list_buffer;
  }
#ifdef FRAMED_QUEUE_SYNC
  sem_destroy(&m_sem);
#endif
  if (m_fd[0])
    close(m_fd[0]);
  if (m_fd[1])
    close(m_fd[1]);
}

int FramedQueue::FramedQueueInit(int size, bool syncsize)
{
  if (syncsize)
  {
    if (-1 == socketpair(PF_UNIX, SOCK_STREAM, 0, m_fd))
    {
      std::cout << "socketpair error:" << strerror(errno) << std::endl;
      return -1;
    }
  }
  if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
    list_buffer = new listBuffer(size);
  else if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
    vector_buffer = new vectorBuffer(size);
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
    ring_buffer = new ringBuffer(size);

  if (list_buffer == nullptr && vector_buffer == nullptr && ring_buffer == nullptr)
  {
    if (m_fd[0])
      close(m_fd[0]);
    if (m_fd[1])
      close(m_fd[1]);
    m_fd[0] = m_fd[1] = 0;
    return -1;
  }
#ifdef FRAMED_QUEUE_SYNC
  sem_init(&m_sem, 0, 10);
#endif
  if (m_fd[0])
    makeSocketNonBlocking(m_fd[0]);
  if (m_fd[1])
    makeSocketNonBlocking(m_fd[1]);
  return 0;
}

int FramedQueue::FramedQueueDataFd()
{
  return m_fd[1];
}

void FramedQueue::FramedQueueWait()
{
#ifdef FRAMED_QUEUE_SYNC
  sem_wait(&m_sem);
#endif
}

int FramedQueue::FramedQueueDataReady()
{
  return 0;
}

void FramedQueue::FramedQueuePost()
{
#ifdef FRAMED_QUEUE_SYNC
  sem_post(&m_sem);
#endif
}

int FramedQueue::FramedQueueDataPut(zpl_uint8 *d, int l)
{
  int ret = 0;
  m_mutex.lock();
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer)
    {
      ret = vector_buffer->vectorBufferDataAdd(d, l);
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer)
    {
      ret = list_buffer->listBufferDataAdd(d, l);
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer)
    {
      ret = ring_buffer->rngBufPut(d, l);
    }
  }
  if (m_fd[0])
  {
    int buf = l;
    write(m_fd[0], &buf, 4);
  }
  m_mutex.unlock();
  return ret;
}

int FramedQueue::FramedQueueDataGet(zpl_uint8 *d, int l)
{
  int ret = 0;
  m_mutex.lock();
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer)
    {
      ret = vector_buffer->vectorBufferDataGet(d, l);
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer)
    {
      ret = list_buffer->listBufferDataGet(d, l);
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer)
    {
      ret = ring_buffer->rngBufGet(d, l);
    }
  }
  m_mutex.unlock();
  return ret;
}

int FramedQueue::FramedQueueDataSizeGet()
{
  if (m_fd[1])
  {
    int buf;
    if (read(m_fd[1], &buf, 4) == 4)
      return buf;
    return -1;
  }
  return 0;
}

int FramedQueue::FramedQueueDataFlush(void)
{
  int ret = 0;
  m_mutex.lock();
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer)
    {
      ret = vector_buffer->vectorBufferDataFlush();
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer)
    {
      ret = list_buffer->listBufferDataFlush();
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer)
    {
      ret = 0;
      ring_buffer->rngFlush();
    }
  }
  while (m_fd[1])
  {
    if (FramedQueueDataSizeGet() == -1)
      break;
  }
  m_mutex.unlock();
  return ret;
}

bool FramedQueue::FramedQueueDataIsFull()
{
  bool ret = 0;
  m_mutex.lock();
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer)
    {
      ret = vector_buffer->vectorBufferDataIsFull();
    }
  }
  if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer)
    {
      ret = list_buffer->listBufferDataIsFull();
    }
  }
  if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer)
    {
      ret = ring_buffer->rngIsFull();
    }
  }
  m_mutex.unlock();
  return ret;
}

bool FramedQueue::FramedQueueDataIsEmpty()
{
  bool ret = 0;
  m_mutex.lock();
  if (FramedQueue::FRAMED_QUEUE_MAP == m_type)
  {
    if (vector_buffer)
    {
      ret = vector_buffer->vectorBufferDataIsEmpty();
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_LIST == m_type)
  {
    if (list_buffer)
    {
      ret = list_buffer->listBufferDataIsEmpty();
    }
  }
  else if (FramedQueue::FRAMED_QUEUE_RING == m_type)
  {
    if (ring_buffer)
    {
      ret = ring_buffer->rngIsEmpty();
    }
  }
  m_mutex.unlock();
  return ret;
}

void FramedQueue::FramedQueueEventTriggerHandler(void *clientData)
{
  ((FramedQueue *)clientData)->event = true;
  std::cout << "FramedQueue::eventTriggerHandler" << std::endl;
}

bool FramedQueue::FramedQueueIsStart()
{
  return event;
}