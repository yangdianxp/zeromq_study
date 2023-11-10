//
// Created by dilei on 2022/8/24.
//

#include <iostream>
#include "ProtocolPackage.h"

PackageDataField::PackageDataField() : strDataBuffer(1024 * 1024 * 2, '\0')
{
    pData = strDataBuffer.data();
}

PackageDataField::~PackageDataField()
{
    pData = nullptr;
}

ProtocolPackage::ProtocolPackage()
{

}

ProtocolPackage::~ProtocolPackage()
{

}

int ProtocolPackage::PackageDataBuffer(std::list<PackageDataField>& listSrcData, std::string& pDest,
                                       std::uint64_t llSendID, std::uint64_t llRcvID, std::uint64_t llSerialNum)
{
    static const int nProtocolHeadSize = SocketProtocolHeadField::GetPackagedSize(); // 协议头
    static const int nPackageHeadSize = SocketPackageHeadField::PackageHeadSize();

    int nMaxBufSize = nProtocolHeadSize + 4; // 最后四个留给CRC
    for (auto& dataField : listSrcData)
    {
        nMaxBufSize += dataField.nSize + nPackageHeadSize;
    }
    nMaxBufSize = std::min(nMaxBufSize, MAX_WRITE_BUFFER_SIZE + MEMORY_ITEM_SIZE);
    pDest.resize(nMaxBufSize);

    char* pDataBuffer = pDest.data();

    SocketProtocolHeadField protocolHeadField;
    protocolHeadField.nSendID = llSendID;
    protocolHeadField.nRecvID = llRcvID;
    protocolHeadField.nMsgSerialNum = llSerialNum;

    int nPackageDataOffset = nProtocolHeadSize;
    while (listSrcData.size() > 0)
    {
        // 判断继续打包大小是否会超出
        PackageDataField& dataField = listSrcData.front();
        if ((nPackageDataOffset + nPackageHeadSize + dataField.nSize + 4) > nMaxBufSize) {
            break;
        }

        // 压入包头数据
        SocketPackageHeadField packageHeadField(protocolHeadField.nPackageCount, dataField.nDataType,
                                                dataField.nSize, protocolHeadField.nMsgSerialNum);
        nPackageDataOffset += packageHeadField.PackagePackageHead(&pDataBuffer[nPackageDataOffset]);

        memcpy(&pDataBuffer[nPackageDataOffset], dataField.pData, dataField.nSize);
        nPackageDataOffset += dataField.nSize;

        ++protocolHeadField.nPackageCount;

        listSrcData.pop_front();
    }

    protocolHeadField.nPackageSize = nPackageDataOffset - nProtocolHeadSize;
    // std::cout << "ProtocolPackage::PackageDataBuffer. nPackageSize: " << protocolHeadField.nPackageSize << std::endl;

    // 封装协议头
    protocolHeadField.PackageProtocolHead(pDataBuffer);
    // 压入CRC值
    memcpy(&pDataBuffer[nPackageDataOffset], &protocolHeadField.nCRCValue, 4);
    nPackageDataOffset += 4;

//    std::list<PackageDataField> listData;
//    std::uint64_t llTempSend, llTempRcv;
   // ProtocolPackage::ParsedPackagedData(llRcvID, pDataBuffer, nPackageDataOffset, listData, llTempSend, llTempRcv);

    return nPackageDataOffset;
}

int ProtocolPackage::ParsedPackagedData(std::uint64_t llProxyID, const char* pData, const int nLength,
                                        std::list<PackageDataField>& listData, std::uint64_t& llSendID, std::uint64_t& llRcvID)
{
    static const int nProtocolHeadSize = SocketProtocolHeadField::GetPackagedSize(); // 协议头
    static const int nPackageHeadSize = SocketPackageHeadField::PackageHeadSize();
    static const int nMinPacketSize = nProtocolHeadSize + nPackageHeadSize + 4;

    int nParsedOffset = 0;

    while (nParsedOffset < nLength)
    {
        int nRemainderSize = nLength - nParsedOffset;
        if (nRemainderSize < nMinPacketSize) { // nMinPacketSize 字节是空数据下的最短报文
            return nParsedOffset;
        }

        SocketProtocolHeadField protocolHeadField;
        int nRet = protocolHeadField.ParsedProtocolHead(&pData[nParsedOffset]);
        if (nRet < 0) {
            // 有可能是脏数据 后移一个字节继续解析
            nParsedOffset += 1;
            continue;
        }

        if (nRemainderSize < (protocolHeadField.nPackageSize + 4 + nRet)) {
            // 后续数据没收全 继续接收
            return nParsedOffset;
        }

        nParsedOffset += nRet;

        // 拿CRC值
        int nCRC = 0;
        memcpy(&nCRC, &pData[nParsedOffset + protocolHeadField.nPackageSize], 4);
        if (nCRC != protocolHeadField.nCRCValue) {
            // 数据有问题 丢弃
            nParsedOffset += protocolHeadField.nPackageSize;
            nParsedOffset += 4;
            std::cout << "ParsedPackagedData PackageSize_2 " << protocolHeadField.nPackageSize
                      << " nParsedOffset " << nParsedOffset << std::endl;
            continue;
        }

        llSendID = protocolHeadField.nSendID;
        llRcvID  = protocolHeadField.nRecvID;

        int nPackagedOffset = nParsedOffset;
        for (int i = 0; i < protocolHeadField.nPackageCount; ++i)
        {
            SocketPackageHeadField dataHeadField;
            nPackagedOffset += dataHeadField.ParsedPackageHead(&pData[nPackagedOffset]);
            if (dataHeadField.nMsgSerialNum != protocolHeadField.nMsgSerialNum) {
                // todo log 收到非法数据 整个数据包丢弃
                std::cout << "ParsedPackagedData PackageSize_5 " << nPackagedOffset << std::endl;
                break;
            }

            PackageDataField dataField;
            dataField.nDataType = dataHeadField.nMsgType;
            dataField.nSize = dataHeadField.nDataSize;
            dataField.llProxyID = protocolHeadField.nSendID;
            memcpy(dataField.pData, &pData[nPackagedOffset], dataHeadField.nDataSize);

            nPackagedOffset += dataHeadField.nDataSize;

            listData.push_back(std::move(dataField));
        }

        nParsedOffset += protocolHeadField.nPackageSize;
        nParsedOffset += 4; // CRC校验完成 需要将长度加上
    }

    return nParsedOffset;
}
