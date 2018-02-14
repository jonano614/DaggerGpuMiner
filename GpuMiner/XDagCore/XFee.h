#pragma once

#include <vector>
#include "XPoolConnection.h"

class XFee
{
private:
    int _nextAddressIndex;
    int _taskCounter;
    bool _connectionIsSwitched;
    std::vector<std::string> _addressList;

    char _poolAddress[256];
    XPoolConnection _connection;
public:
    XFee(std::string& poolAddress);
    ~XFee();

    bool Connect();
    void Disconnect();
    bool ShouldSwitchConnection(XPoolConnection** currentPoolConnection, XPoolConnection* basePoolConnection);
};
