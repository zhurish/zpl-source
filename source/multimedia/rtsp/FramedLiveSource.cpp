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
// Copyright (c) 1996-2024 Live Networks, Inc.  All rights reserved.
// Framed Live Sources
// Implementation

#include "FramedLiveSource.h"
#include <stdlib.h>
#include "auto_include.h"
#include "zplos_include.h"
////////// FramedLiveSource //////////
FramedLiveSource::FramedLiveSource(UsageEnvironment &env, void *queue)
    : FramedSource(env)
{
    frame_queue = queue;
}

FramedLiveSource *FramedLiveSource::createNew(UsageEnvironment &env, void *queue)
{
  FramedLiveSource *newSource = new FramedLiveSource(env, queue);
  return newSource;
}

FramedLiveSource::~FramedLiveSource()
{
}

unsigned FramedLiveSource::maxFrameSize() const
{
  // printf("wangmaxframesize %d %s\n",__LINE__,__FUNCTION__);
  // 这里返回本地h264帧数据的最大长度
  return 1024 * 120;
}

//LiveVideoServerMediaSubssion
void FramedLiveSource::doGetNextFrame()
{
  // 这里读取本地的帧数据，就是一个memcpy(fTo,XX,fMaxSize),要确保你的数据不丢失，即fMaxSize要大于等于本地帧缓存的大小，关键在于上面的maxFrameSize() 虚函数的实现
  zpl_skbuffer_t *skb = zpl_skbqueue_get((zpl_skbqueue_t*)frame_queue);
  if(skb)
  {
    memcpy(fTo, ZPL_SKB_DATA(skb), ZPL_SKB_DATA_LEN(skb));
    fFrameSize = ZPL_SKB_DATA_LEN(skb);

    printf("read dat befor %d %s fMaxSize %d ,fFrameSize %d  \n", __LINE__, __FUNCTION__, fMaxSize, fFrameSize);

    if (fFrameSize == 0)
    {
      handleClosure();
      return;
    }
    // 设置时间戳
    gettimeofday(&fPresentationTime, NULL);
    // Inform the reader that he has data:
    // To avoid possible infinite recursion, we need to return to the event loop to do this:
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *)FramedSource::afterGetting, this);
  }
  return;
}