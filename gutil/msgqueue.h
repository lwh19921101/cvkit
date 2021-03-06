/*
 * This file is part of the Computer Vision Toolkit (cvkit).
 *
 * Author: Heiko Hirschmueller
 *
 * Copyright (c) 2016 Roboception GmbH
 * Copyright (c) 2014 Institute of Robotics and Mechatronics, German Aerospace Center
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GUTIL_MSGQUEUE_H
#define GUTIL_MSGQUEUE_H

#include "semaphore.h"

#include <queue>

namespace gutil
{

/**
  Threadsafe implementation of a queue that can be used for inter-thread
  communication in a producer consumer schema.
*/

template <class T> class MsgQueue
{
  private:

    int           nmax;
    std::queue<T> queue;
    Semaphore     mutex;
    Semaphore     full;
    Semaphore     empty;

  public:

    /**
      Initialization of a message queue.

      @param _nmax Maximum number of messages.
    */

    MsgQueue(int _nmax)
    {
      nmax=std::max(1, _nmax);

      mutex.increment();

      for (int i=0; i<nmax; i++)
      {
        empty.increment();
      }
    }

    /**
      Add a message to the queue. If the queue has reached the maximum message
      count, then this method blocks until pop() is called.

      @param msg Message to be added to the queue.
    */

    void push(const T &msg)
    {
      empty.decrement();

      {
        Lock lock(mutex);
        queue.push(msg);
      }

      full.increment();
    }

    /**
      Removes a message from the queue. If the queue is empty, then this method
      blocks until push() has been called.

      @return Message.
    */

    T pop()
    {
      T ret;

      full.decrement();

      {
        Lock lock(mutex);
        ret=queue.front();
        queue.pop();
      }

      empty.increment();

      return ret;
    }
};

/**
  Threadsafe implementation of a queue with replace, i.e. push never blocks,
  that can be used for inter-thread communication in a producer consumer
  schema.
*/

template <class T> class MsgQueueReplace
{
  private:

    int           nmax;
    std::queue<T> queue;
    Semaphore     mutex;
    Semaphore     full;

  public:

    /**
      Initialization of a message queue.

      @param _nmax Maximum number of messages.
    */

    MsgQueueReplace(int _nmax)
    {
      nmax=std::max(1, _nmax);
      mutex.increment();
    }

    /**
      Add a message to the queue. If the queue has reached the maximum message
      count, then this method removes the oldest message. Thus this method
      never blocks.

      @param msg Message to be added to the queue.
    */

    void push(const T &msg)
    {
      Lock lock(mutex);

      if (queue.size() >= nmax)
      {
        queue.pop();
        queue.push(msg);
      }
      else
      {
        queue.push(msg);
        full.increment();
      }
    }

    /**
      Removes a message from the queue. If the queue is empty, then this method
      blocks until push() has been called.

      @return Message.
    */

    T pop()
    {
      T ret;

      full.decrement();

      {
        Lock lock(mutex);
        ret=queue.front();
        queue.pop();
      }

      return ret;
    }
};

}

#endif
