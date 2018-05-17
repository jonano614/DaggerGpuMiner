#include "XNetwork.h"
#include <stdlib.h>
#include <stdio.h>
#include "Core/Log.h"

#ifdef __linux__
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#elif _WIN32
#include "win\netinet\in.h"
#define poll WSAPoll
#endif

#ifdef _WIN32
#define strtok_r        strtok_s
#define ioctl           ioctlsocket
#define fcntl(a,b,c)    0
#define close           closesocket
#define write(a,b,c)    send(a,b,c,0)
#define read(a,b,c)     recv(a,b,c,0)
#define sysconf(x)      (512)

#else
#define INVALID_SOCKET  -1
#endif

XNetwork::XNetwork()
{
    _socket = INVALID_SOCKET;
}

XNetwork::~XNetwork()
{
    Close();
}

bool XNetwork::Initialize()
{
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#endif
    return true;
}

bool XNetwork::ValidateAddress(const char *address, sockaddr_in &peerAddr)
{
    char *lasts;
    char buf[0x100] = {0};

    // Fill in the address of server
    memset(&peerAddr, 0, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;

    // Resolve the server address (convert from symbolic name to IP number)
    strncpy(buf, address, sizeof(buf));
    char *addressPart = strtok_r(buf, ":", &lasts);
    if(!addressPart)
    {
        //mess = "host is not given";
        return false;
    }
    if(!strcmp(addressPart, "any"))
    {
        peerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if(!inet_aton(addressPart, &peerAddr.sin_addr))
    {
        hostent *host = gethostbyname(addressPart);
        if(!host || !host->h_addr_list[0])
        {
            //mess = "cannot resolve host ", mess1 = poolAddressPart;
            return false;
        }
        // Write resolved IP address of a server to the address structure
        memmove(&peerAddr.sin_addr.s_addr, host->h_addr_list[0], 4);
    }

    // Resolve port
    char *portPart = strtok_r(0, ":", &lasts);
    if(!portPart)
    {
        //mess = "port is not given";
        return false;
    }
    peerAddr.sin_port = htons(atoi(portPart));

    return true;
}

bool XNetwork::Connect(const char *address)
{
    int reuseAddr = 1;
    linger lingerOpt = { 1, 0 }; // Linger active, timeout 0
    sockaddr_in peerAddr;

    if(!ValidateAddress(address, peerAddr))
    {
        return false;
    }

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_socket == INVALID_SOCKET)
    {
        //mess = "cannot create a socket";
        return false;
    }
    if(fcntl(_socket, F_SETFD, FD_CLOEXEC) == -1)
    {
        //TODO: log
        //cheatcoin_err("pool  : can't set FD_CLOEXEC flag on socket %d, %s\n", g_socket, strerror(errno));
    }

    // Set the "LINGER" timeout to zero, to close the listen socket
    // immediately at program termination.
    setsockopt(_socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(int));

    // Now, connect to a pool
    int res = connect(_socket, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
    if(res)
    {
        //mess = "cannot connect to the pool";
        Close();
        return false;
    }
    return true;
}

//TODO: think about exception instead of result failure flag
bool XNetwork::IsReady(NetworkAction action, int timeout, bool& success)
{
    success = false;
    if(action != NetworkAction::Read && action != NetworkAction::Write)
    {
        return false;
    }
    short desiredAction = action == NetworkAction::Read ? POLLIN : POLLOUT;

    pollfd p;
    p.fd = _socket;
    p.events = desiredAction;
    if(!poll(&p, 1, timeout))
    {
        success = true;
        return false;
    }
    if(p.revents & POLLHUP)
    {
        clog(XDag::LogChannel) << "Connection is closed";
        return false;
    }
    if(p.revents & POLLERR)
    {
        clog(XDag::LogChannel) << "Connection error";
        return false;
    }
    success = true;
    return (p.revents & desiredAction) > 0;
}

int XNetwork::Write(char* buf, int len)
{
    return write(_socket, buf, len);
}

int XNetwork::Read(char* buf, int len)
{
    return read(_socket, buf, len);
}

void XNetwork::Close()
{
    if(_socket != INVALID_SOCKET)
    {
        close(_socket);
        _socket = INVALID_SOCKET;
    }
}
