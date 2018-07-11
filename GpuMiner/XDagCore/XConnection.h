// Base network logic
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#ifdef __linux__
#include "netinet/in.h"
#include "sys/socket.h"
typedef int SOCKET;
#elif defined (__APPLE__)|| defined (__MACOS__)
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

class XConnection
{
private:
    SOCKET _socket;

    static bool ValidateAddress(const char *address, sockaddr_in &_peerAddr);
public:
    XConnection();
    virtual ~XConnection();

    virtual bool Initialize();
    virtual bool Connect(const char *address);
    bool IsReady(NetworkAction action, int timeout, bool &success);
    int Write(char* buf, int len);
    int Read(char* buf, int len);
    void Close();

    bool IsConnected() { return _socket != -1; }
};

