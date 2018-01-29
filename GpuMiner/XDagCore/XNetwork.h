#pragma once

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#else
#define SOCKET int
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

