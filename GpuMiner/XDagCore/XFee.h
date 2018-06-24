#pragma once

#include <vector>
#include "XPoolConnection.h"

class XFee
{
private:
    uint32_t _nextAddressIndex;
    uint32_t _taskCounter;
    bool _connectionIsSwitched;
    std::vector<std::string> _addressList;

    char _poolAddress[256];
    XPoolConnection _connection;
    
    bool Connect();
    void Disconnect();
public:
    XFee(std::string& poolAddress);
    ~XFee();
    
    bool SwitchConnection(XPoolConnection** currentPoolConnection, XPoolConnection* basePoolConnection);
    bool ConnectionIsSwitched() const { return _connectionIsSwitched; }
};
