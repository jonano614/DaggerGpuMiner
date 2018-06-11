#include "XFee.h"
#include "Core/Log.h"

#ifdef _DEBUG
#define FEE_PERIOD 4
const std::string GpuDevAddress = "YMB0XWN1vxY5jLiZwSDeTDQTRO2NVEW9";
#else
#define FEE_PERIOD 100
const std::string GpuDevAddress =  "gKNRtSL1pUaTpzMuPMznKw49ILtP6qX3";
const std::string CommunityAddress = "FQglVQtb60vQv2DOWEUL7yh3smtj7g1s";
#endif

XFee::XFee(std::string& poolAddress)
{
    _nextAddressIndex = 0;
    _taskCounter = 0;
    _connectionIsSwitched = false;
    strcpy(_poolAddress, poolAddress.c_str());
#ifdef _DEBUG
    _addressList.push_back(GpuDevAddress);
#else
    _addressList.push_back(GpuDevAddress);
    _addressList.push_back(CommunityAddress);
#endif
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
    return _connection.Connect(_poolAddress);
}

void XFee::Disconnect()
{
    _connection.Close();
}

//increases internal tasks counter and once a 100 tasks switches connection and address for mining (duration - one task)
bool XFee::SwitchConnection(XPoolConnection** currentPoolConnection, XPoolConnection* basePoolConnection)
{
    if(_connectionIsSwitched)
    {
        if(_taskCounter++ == 0)
        {
            return 0;
        }
        _taskCounter = 0;
        *currentPoolConnection = basePoolConnection;
        _connectionIsSwitched = false;
        Disconnect();

        return true;
    }

    if(++_taskCounter < FEE_PERIOD)
    {
        return false;
    }

    _taskCounter = 0;
    if(!Connect())
    {
        return false;
    }
    _connection.SetAddress(_addressList[_nextAddressIndex]);
    *currentPoolConnection = &_connection;
    _connectionIsSwitched = true;
    if(++_nextAddressIndex >= _addressList.size())
    {
        _nextAddressIndex = 0;
    }
    return true;
}
