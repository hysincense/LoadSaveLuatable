 
// The MIT License (MIT)

// Copyright 2010-2017 hysincense
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#ifndef _SSOCK_H__
#define _SSOCK_H__

#if defined(_WIN32)

#include <windows.h>

 
#include <winsock.h>
 

#define DSOCKERR		SOCKET_ERROR
#define DWOULDBLOCK		WSAEWOULDBLOCK
#define DCONNECTERR		WSAEWOULDBLOCK

#define socklen_t long

#else

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DSOCKERR		-1
#define DWOULDBLOCK		EWOULDBLOCK
#define DCONNECTERR		EINPROGRESS

#endif

#include "lua.h"

#define SSOCK_NAMESPACE "ssock"
int eng_lua_ssock(lua_State *L);
#endif
