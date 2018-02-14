#include "XFee.h"
#include "Core/Log.h"

#define FEE_PERIOD 100
const std::string GpuDevAddress = "gKNRtSL1pUaTpzMuPMznKw49ILtP6qX3";
const std::string CommunityAddress = "FQglVQtb60vQv2DOWEUL7yh3smtj7g1s";

XFee::XFee(std::string& poolAddress)
{
    _nextAddressIndex = 0;
    _taskCounter = 0;
    _connectionIsSwitched = false;
    strcpy(_poolAddress, poolAddress.c_str());
    _addressList.push_back(GpuDevAddress);
    _addressList.push_back(CommunityAddress);
}

XFee::~XFee()
{
    Disconnect();
}

bool XFee::Connect()
{
    if(!_connection.Initialize())
    {
        clog(XDag::LogChannel) << "Failed to initialize network connection";
        return false;
    }
    if(!_connection.Connect(_poolAddress))
    {
        return false;
    }
    return true;
}

void XFee::Disconnect()
{
    _connection.Close();
}

//increases internal tasks counter and once a 100 tasks switches connection and address for mining (duration - one task)
bool XFee::ShouldSwitchConnection(XPoolConnection** currentPoolConnection, XPoolConnection* basePoolConnection)
{
    if(_connectionIsSwitched)
    {
        *currentPoolConnection = basePoolConnection;
        _connectionIsSwitched = false;
        return true;
    }

    if(++_taskCounter < FEE_PERIOD)
    {
        return false;
    }

    _taskCounter = 0;
    _connection.SetAddress(_addressList[_nextAddressIndex]);
    *currentPoolConnection = &_connection;
    _connectionIsSwitched = true;
    if(++_nextAddressIndex >= _addressList.size())
    {
        _nextAddressIndex = 0;
    }
    return true;
}