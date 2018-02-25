#pragma once

#ifdef __linux__
#include "netinet/in.h"
#include "sys/socket.h"
typedef int SOCKET;
#elif __MACOS
#include "netinet/in.h"
#include "sys/socket.h"
typedef int SOCKET;
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#endif

enum class NetworkAction
{
    Read,
    Write
};

class XNetwork
{
private:
    SOCKET _socket;

    bool ValidateAddress(const char *address, sockaddr_in &_peerAddr);
public:
    XNetwork();
    virtual ~XNetwork();

    bool Initialize();
    bool Connect(const char *address);
    bool IsReady(NetworkAction action, int timeout, bool &success);
    int Write(char* buf, int len);
    int Read(char* buf, int len);
    void Close();

    bool IsConnected() { return _socket != -1; }
};

