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
#include "ssock.h"
#include "lua.h"
#include "lauxlib.h"

int ff_netstart(int v)
{
#ifdef _WIN32
	struct WSAData wsa;
	WSAStartup((unsigned short)v, &wsa);
#endif
	return 0;
}

int ff_netclose(void)
{
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

int ff_socket(int family, int type, int protocol)
{
	return (int)socket(family, type, protocol);
}

int ff_close(int sock)
{
	if (sock < 0) return 0;
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	return 0;
}

int ff_connect(int sock, const struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return connect(sock, addr, len);
}

int ff_shutdown(int sock, int mode)
{
	return shutdown(sock, mode);
}

int ff_bind(int sock, const struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return bind(sock, addr, len);
}

int ff_listen(int sock, int count)
{
	return listen(sock, count);
}

int ff_accept(int sock, struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return accept(sock, addr, &len);
}

int ff_errno(void)
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

int ff_send(int sock, const void *buf, long size, int mode)
{
	return send(sock, (char*)buf, size, mode);
}

int ff_recv(int sock, void *buf, long size, int mode)
{
	return recv(sock, (char*)buf, size, mode);
}

int ff_sendto(int sock, const void *buf, long size, int mode, const struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return sendto(sock, (char*)buf, size, mode, addr, len);
}

int ff_recvfrom(int sock, void *buf, long size, int mode, struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return recvfrom(sock, (char*)buf, size, mode, addr, &len);
}

int ff_ioctl(int sock, long cmd, unsigned long *argp)
{
#ifdef _WIN32
	return ioctlsocket(sock, cmd, argp);
#else
	return ioctl(sock, cmd, argp);
#endif
}

int ff_setsockopt(int sock, int level, int optname, const char *optval, int optlen)
{
	socklen_t len = optlen;
	return setsockopt(sock, level, optname, optval, len);
}

int ff_getsockopt(int sock, int level, int optname, char *optval, int *optlen)
{
	socklen_t len = (optlen)? *optlen : 0;
	int retval;
	retval = getsockopt(sock, level, optname, optval, &len);
	if (optlen) *optlen = len;
    
	return retval;
}

int ff_sockname(int sock, struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return getsockname(sock, addr, &len);
}

int ff_peername(int sock, struct sockaddr *addr)
{
	socklen_t len = sizeof(struct sockaddr);
	return getpeername(sock, addr, &len);
}
//------------------------------------------------------------------------------------------

int ssock_close(lua_State *L)
{
    int sock = luaL_checkinteger(L, 1);
    if (sock > 0)
        ff_close(sock);
    return 0;
}

int ssock_canWrite(int sock, int sec)
{
    fd_set fdw;
    struct timeval tv;
	int ret;

    FD_ZERO(&fdw);
    FD_SET(sock, &fdw);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    ret = select(sock + 1, NULL, &fdw, NULL, &tv);
    
    if ( ( ret < 0 ) && ( ff_errno() != DCONNECTERR ) )
    {
        return -1;
    }
    return 0;
}

int ssock_canRead(int sock)
{
    fd_set fdr;
    struct timeval tv;
	int ret;

    FD_ZERO(&fdr);
    FD_SET(sock, &fdr);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    ret = select(sock + 1, &fdr, NULL, NULL, &tv);
    
    if ( ret <= 0 || !FD_ISSET(sock, &fdr) )
    {
        return -1;
    }
    return 0;
}

int ssock_connect(lua_State *L)
{
    size_t len;
    const char* name = luaL_checklstring(L, 1, &len);
    int port = luaL_checkinteger(L, 2);
    int sock = -1;
    struct sockaddr_in addr;
    int i;
	unsigned long revalue = 1;

	for (i=0; name[i]; ++i)
	{
		if (!((name[i] >= '0' && name[i] <= '9') || name[i] == '.'))
		{
            return 0;
		}
	}
    
    sock = ff_socket(AF_INET, SOCK_STREAM, 0);
	ff_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&revalue, sizeof(revalue));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    addr.sin_addr.S_un.S_addr = inet_addr(name);
#else
    addr.sin_addr.s_addr = inet_addr(name);
#endif
    
	if ((DSOCKERR == ff_connect(sock, (struct sockaddr *)&addr)) && (DCONNECTERR == ff_errno()))
	{
        if ( ssock_canWrite(sock, 1) < 0 )
        {
            ff_close(sock);
            sock = -1;
            return 0;
        }
	}
    lua_pushinteger(L, sock);
	return 1;
}

int ssock_bind(lua_State *L)
{
    int port = luaL_checkinteger(L, 1);
    int sock = -1;
    struct sockaddr_in addr;
	unsigned long revalue = 1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#else
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    
    sock = ff_socket(AF_INET, SOCK_STREAM, 0);
	ff_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&revalue, sizeof(revalue));
    
	if (ff_bind(sock, (struct sockaddr *)&addr) < 0)
    {
        ff_close(sock);
        sock = -1;
        return 0;
    }
    
    if ( ff_listen(sock, 5) < 0 )
    {
        ff_close(sock);
        sock = -1;
        return 0;
    }
    
    lua_pushinteger(L, sock);
    return 1;
}

int ssock_accept(lua_State *L)
{
    int sock = luaL_checkinteger(L, 1);
    int block = lua_toboolean(L, 2);
	int client = -1;
	struct sockaddr_in addr;

    if ( sock < 0 )
    {
        return 0;
    }
    
    if ( !block && ssock_canRead(sock) < 0 )
    {
        return 0;
    }
        
	client = ff_accept(sock, (struct sockaddr *)&addr);
        
    lua_pushinteger(L, client);
    return 1;
}

int ssock_send(lua_State *L)
{
    size_t len;
    int sock = luaL_checkinteger(L, 1);
    const char* str = luaL_checklstring(L, 2, &len);
    int strLength = strlen(str);
    int total_send = 0;
    char tempBuffer[256] = {0};
    
    if (str) {
        tempBuffer[0] = strLength;
        strncpy(&tempBuffer[1], str, 255);
        strLength = strLength + 1;
    }
    
    if ( sock < 0 )
    {
        lua_pushinteger(L, 0);
        return 1;
    }
    
    len = -1;
    
	do {
		len = ff_send( sock, &tempBuffer[ total_send ], strLength - total_send, 0 );
		if ( len > 0 ) total_send += len;
	} while( len > 0 && total_send < strLength );
    
	lua_pushinteger(L, total_send - 1);
    return 1;
}

int ssock_recv(lua_State *L)
{
    int sock = luaL_checkinteger(L, 1);
    int block = lua_toboolean(L, 2);
    char length = -1;
    char buffer[256] = {0};
    int len;
    int total_recv = 0;
    int disconnect = 0;
    
    if ( sock < 0 )
    {
        return 0;
    }

    if ( !block && ssock_canRead(sock) < 0 )
    {
        return 0;
    }
    
    len = ff_recv(sock, &length, 1, 0);
    
    if ((!len) || ((len == -1) && (ff_errno() != DWOULDBLOCK)))
	{
		disconnect = 1;
	}
    
    if ( len == 1 )
    {
        do {
            len = ff_recv( sock, &buffer[ total_recv ], length - total_recv, 0 );	
            if ( len > 0 ) total_recv += len;
        } while( len > 0 && total_recv < length );
        
        if ((!len) || ((len == -1) && (ff_errno() != DWOULDBLOCK)))
        {
            disconnect = 1;
        }
    }

    if ( disconnect )
    {
        lua_pushnil(L);
        lua_pushstring(L, "disconnected");
        return 2;
    }
    else
    {
        lua_pushstring(L, buffer);
        return 1;
    }
}

//------------------------------------------------------------------------------------------

static const struct luaL_Reg emptyfuns[] = {
    {NULL, NULL}
};

static const struct luaL_Reg funs[] = {
	{"bind", ssock_bind },
	{"accept", ssock_accept },	
    {"connect", ssock_connect},
    {"send", ssock_send},
    {"recv", ssock_recv},
	{"close", ssock_close },
	{NULL, NULL}
};

int ext_lua_ssock(lua_State *L)
{
	luaL_newmetatable(L, SSOCK_NAMESPACE);
	luaL_register(L, NULL, emptyfuns);
	luaL_register(L, SSOCK_NAMESPACE, funs);    
    lua_pop(L, 2); 
    return 0;
}
