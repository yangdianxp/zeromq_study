//
// Created by dilei on 2022/8/24.
//

#ifndef DRONEPROJECT_PROTOCOLPACKAGE_H
#define DRONEPROJECT_PROTOCOLPACKAGE_H


#include <string.h>
#include <sys/time.h>
#include <string>
#include <list>
#include "DroneSocketDefine.h"

// 网络协议头
struct SocketProtocolHeadField
{
    ProxyIDType nSendID;       // 发送方标识
    ProxyIDType nRecvID;       // 接收方标识
    char strVersion[6];          // 版本号
    ProxyIDType nMsgSerialNum; // 消息序列号
    long nTimestamp;   // 时间戳

    int nPackageCount;   // 包总数
    int nPackageSize;    // 数据总大小(包括数据包包头,不包含协议头)
    int nCRCValue;       // 校验值

    SocketProtocolHeadField()
    {
        memset(this, 0, sizeof(SocketProtocolHeadField));

        nCRCValue = DRONE_SOCKET_CRC_VALUE;
        strcpy(strVersion, DRONE_SOCKET_PROXY_VERSION);
    }

    static int GetPackagedSize()
    {
        return 8 + 8 + 6 + 8 + 8 + 4 + 4 + 4;
    }

    int PackageProtocolHead(char* pBuffer)
    {
        int nOffset = 0;
        memcpy(&pBuffer[nOffset], &nSendID, 8);
        nOffset += 8;

        memcpy(&pBuffer[nOffset], &nRecvID, 8);
        nOffset += 8;

        memcpy(&pBuffer[nOffset], strVersion, 6);
        nOffset += 6;

        memcpy(&pBuffer[nOffset], &nMsgSerialNum, 8);
        nOffset += 8;

#ifdef WIN32
        time_t tt = clock();
        nTimestamp = tt;
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        nTimestamp = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

        memcpy(&pBuffer[nOffset], &nTimestamp, 8);
        nOffset += 8;

        memcpy(&pBuffer[nOffset], &nPackageCount, 4);
        nOffset += 4;

        memcpy(&pBuffer[nOffset], &nPackageSize, 4);
        nOffset += 4;

        memcpy(&pBuffer[nOffset], &nCRCValue, 4);
        nOffset += 4;

        return nOffset;
    }

    int ParsedProtocolHead(const char* pPackage)
    {
        int nOffset = 0;

        memcpy(&nSendID, &pPackage[nOffset], 8);
        nOffset += 8;

        memcpy(&nRecvID, &pPackage[nOffset], 8);
        nOffset += 8;

        memcpy(strVersion, &pPackage[nOffset], 6);
        nOffset += 6;

        if (0 != strcmp(strVersion, DRONE_SOCKET_PROXY_VERSION)) {
            return -1;
        }

        memcpy(&nMsgSerialNum, &pPackage[nOffset], 8);
        nOffset += 8;

        memcpy(&nTimestamp, &pPackage[nOffset], 8);
        nOffset += 8;

        memcpy(&nPackageCount, &pPackage[nOffset], 4);
        nOffset += 4;

        memcpy(&nPackageSize, &pPackage[nOffset], 4);
        nOffset += 4;

        memcpy(&nCRCValue, &pPackage[nOffset], 4);
        nOffset += 4;

        return nOffset;
    }
};

// 数据包包头
struct SocketPackageHeadField
{
    int nSerialNum;    // 序号
    int nMsgType;
    int nDataSize;     // 包数据大小(不包含包头)
    std::uint64_t nMsgSerialNum; // 消息序列号

    SocketPackageHeadField()
            : nSerialNum(0)
            , nMsgType(0)
            , nDataSize(0)
            , nMsgSerialNum(0)
    {

    }

    SocketPackageHeadField(int nNum, int nMessageType, int nSize,  std::uint64_t nSerial)
        : nSerialNum(nNum)
        , nMsgType(nMessageType)
        , nDataSize(nSize)
        , nMsgSerialNum(nSerial)
    {

    }

    static int PackageHeadSize()
    {
        return (4 + 4 + 4 + 8);
    }

    int PackagePackageHead(char* pBuffer)
    {
        int nOffset = 0;
        memcpy(&pBuffer[nOffset], &nSerialNum, 4);
        nOffset += 4;

        memcpy(&pBuffer[nOffset], &nMsgType, 4);
        nOffset += 4;

        memcpy(&pBuffer[nOffset], &nDataSize, 4);
        nOffset += 4;

        memcpy(&pBuffer[nOffset], &nMsgSerialNum, 8);
        nOffset += 8;

        return nOffset;
    }

    int ParsedPackageHead(const char* pPackage)
    {
        int nOffset = 0;
        memcpy(&nSerialNum, &pPackage[nOffset], 4);
        nOffset += 4;

        memcpy(&nMsgType, &pPackage[nOffset], 4);
        nOffset += 4;

        memcpy(&nDataSize, &pPackage[nOffset], 4);
        nOffset += 4;

        memcpy(&nMsgSerialNum, &pPackage[nOffset], 8);
        nOffset += 8;

        return nOffset;
    }
};

struct PackageDataField
{
    int nDataType = 0;
    std::string strDataBuffer;
    char* pData = nullptr;
    int nSize = 0;
    ProxyIDType llProxyID = 0;

    PackageDataField();
    PackageDataField(const PackageDataField& other) noexcept
    {
        this->nDataType = other.nDataType;
        this->pData = other.pData;
        this->nSize = other.nSize;
        this->llProxyID = other.llProxyID;
    }

    PackageDataField(PackageDataField&& other) noexcept
    {
        this->nDataType = other.nDataType;
        this->pData = other.pData;
        this->nSize = other.nSize;
        this->llProxyID = other.llProxyID;

        other.nDataType = 0;
        other.pData = nullptr;
        other.nSize = 0;
        other.llProxyID = 0;
    }

    PackageDataField& operator=(const PackageDataField& other) noexcept
    {
        if (this != &other) {
            this->nDataType = other.nDataType;
            this->pData = other.pData;
            this->nSize = other.nSize;
            this->llProxyID = other.llProxyID;
        }

        return *this;
    }

    PackageDataField& operator=(PackageDataField&& other)
    {
        if (this != &other) {
            this->nDataType = other.nDataType;
            this->pData = other.pData;
            this->nSize = other.nSize;
            this->llProxyID = other.llProxyID;

            other.nDataType = 0;
            other.pData = nullptr;
            other.nSize = 0;
            other.llProxyID = 0;
        }

        return *this;
    }

    ~PackageDataField();
};

class ProtocolPackage
{
public:
    ProtocolPackage();
    virtual ~ProtocolPackage();

    /* |__ProtoHead__|__PackageHead__|__PackageData__|__---__|__CRC__| */
    static int PackageDataBuffer(std::list<PackageDataField>& listSrcData, std::string& pDest,
                                 std::uint64_t llSendID, std::uint64_t llRcvID, std::uint64_t llSerialNum);

    static int ParsedPackagedData(std::uint64_t llProxyID, const char* pData, const int nLength,
                                  std::list<PackageDataField>& listData, std::uint64_t& llSendID, std::uint64_t& llRcvID);
};


#endif //DRONEPROJECT_PROTOCOLPACKAGE_H
