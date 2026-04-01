#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include "CRC32.h"

/**
 * @brief 通信报文协议类
 * 严格遵循 Q/GDW XXXXX.3-2022 通信报文通用格式
 */
class CommunicationProtocol
{
public:
    // ==================== 字段定义（与表格完全对应）====================
    
    // 1. 报文头 (uint32, 4 字节) - 固定值 0xEB90EB90
    uint32_t packetHeader;
    
    // 2. 版本号 (uint8, 1 字节) - 从 1 开始
    uint8_t version;
    
    // 3. 序号 (uint16, 2 字节) - 范围 [1, 65535]，循环使用
    uint16_t sequenceNumber;
    
    // 4. 请求标志 (uint8, 1 字节) - 请求:0x00, 响应:0x01
    uint8_t requestFlag;
    
    // 5. 数据包总长度 (uint64, 8 字节) - 包含包头包尾的总长度
    uint64_t totalPacketLength;
    
    // 6. 报文类型编码 (uint32, 4 字节) - 标识业务类型
    uint32_t packetTypeCode;
    
    // 7. 压缩标志 (uint8, 1 字节) - 无压缩:0x00, gzip 压缩:0x01
    uint8_t compressionFlag;
    
    // 8. 加密标志 (uint8, 1 字节) - 无加密:0x00, 加密:0x01
    uint8_t encryptionFlag;
    
    // 9. 仪器厂商 (uint8, 1 字节) - FLIR:0x05, 其他:0x00
    uint8_t instrumentVendor;
    
    // 10. 备用 (15 字节) - 默认填 0
    std::vector<uint8_t> reserved;
    
    // 11. 业务数据格式 (uint8, 1 字节) - XML:0x01
    uint8_t serviceDataFormat;
    
    // 12. 业务数据长度 (uint64, 8 字节) - UTF-8 编码的总长度
    uint64_t serviceDataLength;
    
    // 13. 业务数据 (char 数组，N 字节) - UTF-8 编码
    std::vector<uint8_t> serviceData;
    
    // 14. 检测数据文件长度 (uint64, 8 字节) - 不传文件则为 0
    uint64_t detectionFileLength;
    
    // 15. 检测数据文件 (自定义，M 字节) - .dat 文件二进制数据
    std::vector<uint8_t> detectionFileData;

    // 16. 校验字节 (uint32, 4 字节) - CRC32 校验
    uint32_t crc32Checksum;
    
    // 17. 报文尾 (uint8, 1 字节) - 固定值 0x03
    uint8_t packetTail;
    
    // ==================== 构造函数 ====================
    
    CommunicationProtocol()
    {
        // 初始化默认值
        packetHeader = 0xEB90EB90;
        version = 1;
        sequenceNumber = 1;
        requestFlag = 0x00;  // 默认为请求
        totalPacketLength = 0;
        packetTypeCode = 0;
        compressionFlag = 0x00;  // 无压缩
        encryptionFlag = 0x00;   // 无加密
        instrumentVendor = 0x00; // 其他厂家
        reserved.resize(15, 0);  // 备用 15 字节填 0
        serviceDataFormat = 0x01; // XML 格式
        serviceDataLength = 0;
        detectionFileLength = 0;
        packetTail = 0x03;
    }
    
    // ==================== 构建方法 ====================
    
    /**
     * @brief 从字节流构建协议对象
     * @param data 完整的字节流数据
     * @param length 数据长度
     * @return 是否成功解析
     */
    bool buildFromBytes(const uint8_t* data, size_t length)
    {
        if (length < 42) // 最小长度（不含业务数据和检测数据）
        {
            return false;
        }
        
        size_t offset = 0;
        
        // 1. 报文头 (4 字节)
        if (length < offset + 4) return false;
        packetHeader = readUint32(data, offset);
        offset += 4;
        
        // 2. 版本号 (1 字节)
        if (length < offset + 1) return false;
        version = data[offset];
        offset += 1;
        
        // 3. 序号 (2 字节)
        if (length < offset + 2) return false;
        sequenceNumber = readUint16(data, offset);
        offset += 2;
        
        // 4. 请求标志 (1 字节)
        if (length < offset + 1) return false;
        requestFlag = data[offset];
        offset += 1;
        
        // 5. 数据包总长度 (8 字节)
        if (length < offset + 8) return false;
        totalPacketLength = readUint64(data, offset);
        offset += 8;
        
        // 6. 报文类型编码 (4 字节)
        if (length < offset + 4) return false;
        packetTypeCode = readUint32(data, offset);
        offset += 4;
        
        // 7. 压缩标志 (1 字节)
        if (length < offset + 1) return false;
        compressionFlag = data[offset];
        offset += 1;
        
        // 8. 加密标志 (1 字节)
        if (length < offset + 1) return false;
        encryptionFlag = data[offset];
        offset += 1;
        
        // 9. 仪器厂商 (1 字节)
        if (length < offset + 1) return false;
        instrumentVendor = data[offset];
        offset += 1;
        
        // 10. 备用 (15 字节)
        if (length < offset + 15) return false;
        reserved.assign(data + offset, data + offset + 15);
        offset += 15;
        
        // 11. 业务数据格式 (1 字节)
        if (length < offset + 1) return false;
        serviceDataFormat = data[offset];
        offset += 1;
        
        // 12. 业务数据长度 (8 字节)
        if (length < offset + 8) return false;
        serviceDataLength = readUint64(data, offset);
        offset += 8;
        
        // 13. 业务数据 (N 字节)
        if (length < offset + serviceDataLength) return false;
        serviceData.assign(data + offset, data + offset + serviceDataLength);
        offset += serviceDataLength;
        
        // 14. 检测数据文件长度 (8 字节)
        if (length < offset + 8) return false;
        detectionFileLength = readUint64(data, offset);
        offset += 8;
        
        // 15. 检测数据文件 (M 字节)
        if (detectionFileLength > 0)
        {
            if (length < offset + detectionFileLength) return false;
            detectionFileData.assign(data + offset, data + offset + detectionFileLength);
            offset += detectionFileLength;
        }
        
        // 16. 校验字节 (4 字节)
        if (length < offset + 4) return false;
        crc32Checksum = readUint32(data, offset);
        offset += 4;
        
        // 17. 报文尾 (1 字节)
        if (length < offset + 1) return false;
        packetTail = data[offset];
        offset += 1;
        
        return true;
    }
    
    /**
     * @brief 从字节流构建协议对象（vector 版本）
     */
    bool buildFromBytes(const std::vector<uint8_t>& data)
    {
        return buildFromBytes(data.data(), data.size());
    }
    
    /**
     * @brief 从十六进制字符串构建协议对象
     * @param hexString 十六进制字符串（例如："eb90eb90010001..."）
     * @return 是否成功解析
     */
    bool buildFromHexString(const std::string& hexString)
    {
        // 检查字符串长度（必须是偶数）
        if (hexString.length() % 2 != 0)
        {
            return false;
        }
        
        // 将十六进制字符串转换为字节数组
        std::vector<uint8_t> data;
        data.reserve(hexString.length() / 2);
        
        for (size_t i = 0; i < hexString.length(); i += 2)
        {
            std::string byteString = hexString.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
            data.push_back(byte);
        }
        
        // 调用 buildFromBytes
        return buildFromBytes(data);
    }
    
    /**
     * @brief 从十六进制字符串构建协议对象（支持带空格的字符串）
     * @param hexString 十六进制字符串（例如："eb 90 eb 90 01 00..." 或 "EB90EB90 0100..."）
     * @return 是否成功解析
     */
    bool buildFromHexStringWithSpaces(const std::string& hexString)
    {
        // 移除所有空格和分隔符
        std::string cleanHex;
        for (char c : hexString)
        {
            if (c != ' ' && c != '-' && c != ':' && c != '\t' && c != '\n' && c != '\r')
            {
                cleanHex += static_cast<char>(std::toupper(c));
            }
        }
        
        return buildFromHexString(cleanHex);
    }
    
    /**
     * @brief 将协议对象转换为十六进制字符串
     * @return 十六进制字符串（不含空格）
     */
    std::string toHexString()
    {
        std::vector<uint8_t> bytes = toBytes();
        std::string hexString;
        hexString.reserve(bytes.size() * 2);
        
        for (size_t i = 0; i < bytes.size(); i++)
        {
            char buffer[3];
            snprintf(buffer, sizeof(buffer), "%02x", bytes[i]);
            hexString += buffer;
        }
        
        return hexString;
    }
    
    /**
     * @brief 将协议对象转换为带空格的十六进制字符串
     * @param separator 分隔符（默认为空格）
     * @return 带分隔符的十六进制字符串（例如："eb 90 eb 90 01 00..."）
     */
    std::string toHexStringWithSeparator(char separator = ' ')
    {
        std::vector<uint8_t> bytes = toBytes();
        std::string hexString;
        hexString.reserve(bytes.size() * 3);
        
        for (size_t i = 0; i < bytes.size(); i++)
        {
            if (i > 0) hexString += separator;
            char buffer[3];
            snprintf(buffer, sizeof(buffer), "%02x", bytes[i]);
            hexString += buffer;
        }
        
        return hexString;
    }
    
    /**
     * @brief 设置业务数据（字符串）
     */
    void setServiceData(const std::string& data)
    {
        serviceData.assign(data.begin(), data.end());
        serviceDataLength = data.length();
    }
    
    /**
     * @brief 设置业务数据（字节数组）
     */
    void setServiceData(const uint8_t* data, size_t length)
    {
        serviceData.assign(data, data + length);
        serviceDataLength = length;
    }
    
    /**
     * @brief 设置检测数据文件
     */
    void setDetectionFile(const uint8_t* data, size_t length)
    {
        detectionFileData.assign(data, data + length);
        detectionFileLength = length;
    }
    
    /**
     * @brief 设置检测数据文件（vector 版本）
     */
    void setDetectionFile(const std::vector<uint8_t>& data)
    {
        detectionFileData = data;
        detectionFileLength = data.size();
    }
    
    // ==================== 序列化方法 ====================
    
    /**
     * @brief 生成完整的字节流（包含 CRC32 校验）
     * @return 包含所有字段数据的字节流，可用于 TCP 传输
     */
    std::vector<uint8_t> toBytes()
    {
        std::vector<uint8_t> buffer;
        
        // 计算总长度（先不计算 CRC32）
        // 固定头部：4+1+2+1+8+4+1+1+1+15+1+8 = 47 字节
        // 业务数据：serviceDataLength
        // 检测文件：detectionFileLength
        // CRC32: 4 字节
        // 包尾：1 字节
        totalPacketLength = 47 + serviceDataLength + detectionFileLength + 4 + 1;
        
        // 1. 报文头 (4 字节)
        writeUint32(buffer, packetHeader);
        
        // 2. 版本号 (1 字节)
        buffer.push_back(version);
        
        // 3. 序号 (2 字节)
        writeUint16(buffer, sequenceNumber);
        
        // 4. 请求标志 (1 字节)
        buffer.push_back(requestFlag);
        
        // 5. 数据包总长度 (8 字节)
        writeUint64(buffer, totalPacketLength);
        
        // 6. 报文类型编码 (4 字节)
        writeUint32(buffer, packetTypeCode);
        
        // 7. 压缩标志 (1 字节)
        buffer.push_back(compressionFlag);
        
        // 8. 加密标志 (1 字节)
        buffer.push_back(encryptionFlag);
        
        // 9. 仪器厂商 (1 字节)
        buffer.push_back(instrumentVendor);
        
        // 10. 备用 (15 字节)
        for (size_t i = 0; i < 15; i++)
        {
            buffer.push_back(reserved[i]);
        }
        
        // 11. 业务数据格式 (1 字节)
        buffer.push_back(serviceDataFormat);
        
        // 12. 业务数据长度 (8 字节)
        writeUint64(buffer, serviceDataLength);
        
        // 13. 业务数据 (N 字节)
        for (size_t i = 0; i < serviceDataLength; i++)
        {
            buffer.push_back(serviceData[i]);
        }
        
        // 14. 检测数据文件长度 (8 字节)
        writeUint64(buffer, detectionFileLength);
        
        // 15. 检测数据文件 (M 字节)
        for (size_t i = 0; i < detectionFileLength; i++)
        {
            buffer.push_back(detectionFileData[i]);
        }
        
        // 16. 校验字节 (4 字节) - 计算 CRC32
        // CRC 范围：从报文头至检测数据文件部分（不含 CRC 本身和包尾）
        crc32Checksum = calc_packet_crc(buffer.data(), buffer.size());
        writeUint32(buffer, crc32Checksum);
        
        // 17. 报文尾 (1 字节)
        buffer.push_back(packetTail);
        
        return buffer;
    }
    
    /**
     * @brief 获取业务数据字符串
     */
    std::string getServiceDataString() const
    {
        return std::string(serviceData.begin(), serviceData.end());
    }
    
    // ==================== 辅助方法 ====================
    
    /**
     * @brief 验证 CRC32 校验和
     * @return 是否校验通过
     */
    bool verifyCRC32() const
    {
        std::vector<uint8_t> data = toBytesWithoutCRC();
        uint32_t calculatedCRC = crc32(data.data(), data.size());
        return calculatedCRC == crc32Checksum;
    }
    
    /**
     * @brief 生成不含 CRC 和包尾的字节流（用于 CRC 计算）
     */
    std::vector<uint8_t> toBytesWithoutCRC() const
    {
        std::vector<uint8_t> buffer;
        
        // 1. 报文头 (4 字节)
        writeUint32(buffer, packetHeader);
        
        // 2. 版本号 (1 字节)
        buffer.push_back(version);
        
        // 3. 序号 (2 字节)
        writeUint16(buffer, sequenceNumber);
        
        // 4. 请求标志 (1 字节)
        buffer.push_back(requestFlag);
        
        // 5. 数据包总长度 (8 字节)
        writeUint64(buffer, totalPacketLength);
        
        // 6. 报文类型编码 (4 字节)
        writeUint32(buffer, packetTypeCode);
        
        // 7. 压缩标志 (1 字节)
        buffer.push_back(compressionFlag);
        
        // 8. 加密标志 (1 字节)
        buffer.push_back(encryptionFlag);
        
        // 9. 仪器厂商 (1 字节)
        buffer.push_back(instrumentVendor);
        
        // 10. 备用 (15 字节)
        for (size_t i = 0; i < 15; i++)
        {
            buffer.push_back(reserved[i]);
        }
        
        // 11. 业务数据格式 (1 字节)
        buffer.push_back(serviceDataFormat);
        
        // 12. 业务数据长度 (8 字节)
        writeUint64(buffer, serviceDataLength);
        
        // 13. 业务数据 (N 字节)
        for (size_t i = 0; i < serviceDataLength; i++)
        {
            buffer.push_back(serviceData[i]);
        }
        
        // 14. 检测数据文件长度 (8 字节)
        writeUint64(buffer, detectionFileLength);
        
        // 15. 检测数据文件 (M 字节)
        for (size_t i = 0; i < detectionFileLength; i++)
        {
            buffer.push_back(detectionFileData[i]);
        }
        
        return buffer;
    }
    
private:
    // ==================== 辅助读写函数（大端模式）====================
    
    static uint16_t readUint16(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<uint16_t>(data[offset]) << 8) | 
               static_cast<uint16_t>(data[offset + 1]);
    }
    
    static uint32_t readUint32(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<uint32_t>(data[offset]) << 24) | 
               (static_cast<uint32_t>(data[offset + 1]) << 16) | 
               (static_cast<uint32_t>(data[offset + 2]) << 8) | 
               static_cast<uint32_t>(data[offset + 3]);
    }
    
    static uint64_t readUint64(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<uint64_t>(data[offset]) << 56) | 
               (static_cast<uint64_t>(data[offset + 1]) << 48) | 
               (static_cast<uint64_t>(data[offset + 2]) << 40) | 
               (static_cast<uint64_t>(data[offset + 3]) << 32) | 
               (static_cast<uint64_t>(data[offset + 4]) << 24) | 
               (static_cast<uint64_t>(data[offset + 5]) << 16) | 
               (static_cast<uint64_t>(data[offset + 6]) << 8) | 
               static_cast<uint64_t>(data[offset + 7]);
    }
    
    static void writeUint16(std::vector<uint8_t>& buffer, uint16_t value)
    {
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeUint32(std::vector<uint8_t>& buffer, uint32_t value)
    {
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeUint64(std::vector<uint8_t>& buffer, uint64_t value)
    {
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((value >> 56) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 48) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 40) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 32) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
};
