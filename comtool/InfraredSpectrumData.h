#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

/**
 * @brief 红外图谱数据格式解析类
 * 根据 Q/GDW 标准红外图谱数据格式定义（表 12）
 * 
 * 数据格式说明:
 * - 所有多字节整数类型均采用大端模式（Big-Endian）
 * - 字符串采用 UNICODE 编码或 ASCII 编码
 * - 字节顺序严格按照定义顺序排列
 */
class InfraredSpectrumData
{
public:
    // ==================== 字段定义（严格按照表 12 顺序）====================
    
    // 1. 检测数据类型编码 (uint8, 1 字节) [0:0] - 必备
    uint8_t dataTypeCode;
    
    // 2. 图谱数据长度 (int32, 4 字节) [1:4] - 必备
    int32_t dataLength;
    
    // 3. 图谱生成时间 (int64, 8 字节) [5:12] - 必备
    // 格式：YYYYMMDDhhmmssfff (年月日时分秒毫秒)
    int64_t createTime;
    
    // 4. 图谱性质 (uint8, 1 字节) [13:13] - 可选
    // 0x03:检测设备单相整体图谱
    // 0x04:检测设备注意相局部图谱
    // 0x05:检测设备所属三相设备图谱
    uint8_t spectrumProperty;
    
    // 5. 被检测设备名称 (char, 118 字节) [14:131] - 可选
    // UNICODE 编码，0x0000 结尾
    std::vector<uint8_t> deviceName;
    
    // 6. 被检测设备编码 (char, 42 字节) [132:173] - 可选
    // ASCII 编码，\0 结尾
    std::vector<uint8_t> deviceCode;
    
    // 7. 测点名称 (char, 128 字节) [174:301] - 必备
    // UNICODE 编码，不够长度以 0x0000 补充
    std::vector<uint8_t> measurementPointName;
    
    // 8. 测点编码 (char, 32 字节) [302:333] - 必备
    // ASCII 编码，\0 结尾
    std::vector<uint8_t> measurementPointCode;
    
    // 9. 检测通道标志 (int16, 2 字节) [334:335] - 可选
    // 仪器的检测通道标识，例如：1
    int16_t detectionChannelFlag;
    
    // 10. 存储器数据类型 (uint8, 1 字节) [336:336] - 必备
    uint8_t storageDataType;
    
    // 11. 温度单位 (uint8, 1 字节) [337:337] - 必备
    // 符合 Q/GDW XXXX.1-XXXX 表 2 要求
    uint8_t temperatureUnit;
    
    // 12. 温度点阵宽度 w (int32, 4 字节) [338:341] - 必备
    // 红外温度点阵的宽度
    int32_t temperatureMatrixWidth;
    
    // 13. 温度点阵高度 h (int32, 4 字节) [342:345] - 必备
    // 红外温度点阵的高度
    int32_t temperatureMatrixHeight;
    
    // 14. 可见光照片数据长度 L1 (int32, 4 字节) [346:349] - 必备
    // 如无可见光照片，该数据为 0
    int32_t visibleLightDataLength;
    
    // 15. 红外照片数据长度 L2 (int32, 4 字节) [350:353] - 必备
    // 如无红外照片，该数据为 0
    int32_t infraredPhotoDataLength;
    
    // 16. 辐射率 (float, 4 字节) [354:357] - 必备
    // 范围：[0~1.00]
    float emissivity;
    
    // 17. 测试距离 (float, 4 字节) [358:361] - 必备
    // 单位：m, 范围：[0, 50000]
    float testDistance;
    
    // 18. 大气温度 (float, 4 字节) [362:365] - 必备
    // 单位：摄氏度
    float atmosphericTemperature;
    
    // 19. 相对湿度 (int8, 1 字节) [366:366] - 必备
    // 范围：[0, 100], 单位%
    int8_t relativeHumidity;
    
    // 20. 反射温度 (float, 4 字节) [367:370] - 必备
    // 单位：摄氏度
    float reflectedTemperature;
    
    // 21. 温宽上限 (float, 4 字节) [371:374] - 必备
    // 单位：温度单位
    float temperatureRangeUpper;
    
    // 22. 温宽下限 (float, 4 字节) [375:378] - 必备
    // 单位：温度单位
    float temperatureRangeLower;
    
    // 23. 可见光照片数据 (可变长度) [379:379+L1-1] - 必备
    std::vector<uint8_t> visibleLightPhotoData;
    
    // 24. 红外照片数据 (可变长度) [379+L1:379+L1+L2-1] - 必备
    std::vector<uint8_t> infraredPhotoData;
    
    // ==================== 构造函数 ====================
    
    InfraredSpectrumData()
    {
        // 初始化默认值
        dataTypeCode = 0;
        dataLength = 0;
        createTime = 0;
        spectrumProperty = 0;
        detectionChannelFlag = 0;
        storageDataType = 0;
        temperatureUnit = 0;
        temperatureMatrixWidth = 0;
        temperatureMatrixHeight = 0;
        visibleLightDataLength = 0;
        infraredPhotoDataLength = 0;
        emissivity = 0.0f;
        testDistance = 0.0f;
        atmosphericTemperature = 0.0f;
        relativeHumidity = 0;
        reflectedTemperature = 0.0f;
        temperatureRangeUpper = 0.0f;
        temperatureRangeLower = 0.0f;
    }
    
    // ==================== 解析方法 ====================
    
    /**
     * @brief 从字节流解析红外图谱数据
     * @param data 完整的字节流数据（detectionFileData）
     * @param length 数据长度
     * @return 是否成功解析
     */
    bool parseFromBytes(const uint8_t* data, size_t length)
    {
        if (length < 379) // 最小长度（不含照片数据）
        {
            return false;
        }
        
        size_t offset = 0;
        
        // 1. 检测数据类型编码 (1 字节) [0:0]
        if (length < offset + 1) return false;
        dataTypeCode = data[offset];
        offset += 1;
        
        // 2. 图谱数据长度 (4 字节) [1:4]
        if (length < offset + 4) return false;
        dataLength = readInt32(data, offset);
        offset += 4;
        
        // 3. 图谱生成时间 (8 字节) [5:12]
        if (length < offset + 8) return false;
        createTime = readInt64(data, offset);
        offset += 8;
        
        // 4. 图谱性质 (1 字节) [13:13]
        if (length < offset + 1) return false;
        spectrumProperty = data[offset];
        offset += 1;
        
        // 5. 被检测设备名称 (118 字节) [14:131]
        if (length < offset + 118) return false;
        deviceName.assign(data + offset, data + offset + 118);
        offset += 118;
        
        // 6. 被检测设备编码 (42 字节) [132:173]
        if (length < offset + 42) return false;
        deviceCode.assign(data + offset, data + offset + 42);
        offset += 42;
        
        // 7. 测点名称 (128 字节) [174:301]
        if (length < offset + 128) return false;
        measurementPointName.assign(data + offset, data + offset + 128);
        offset += 128;
        
        // 8. 测点编码 (32 字节) [302:333]
        if (length < offset + 32) return false;
        measurementPointCode.assign(data + offset, data + offset + 32);
        offset += 32;
        
        // 9. 检测通道标志 (2 字节) [334:335]
        if (length < offset + 2) return false;
        detectionChannelFlag = readInt16(data, offset);
        offset += 2;
        
        // 10. 存储器数据类型 (1 字节) [336:336]
        if (length < offset + 1) return false;
        storageDataType = data[offset];
        offset += 1;
        
        // 11. 温度单位 (1 字节) [337:337]
        if (length < offset + 1) return false;
        temperatureUnit = data[offset];
        offset += 1;
        
        // 12. 温度点阵宽度 (4 字节) [338:341]
        if (length < offset + 4) return false;
        temperatureMatrixWidth = readInt32(data, offset);
        offset += 4;
        
        // 13. 温度点阵高度 (4 字节) [342:345]
        if (length < offset + 4) return false;
        temperatureMatrixHeight = readInt32(data, offset);
        offset += 4;
        
        // 14. 可见光照片数据长度 (4 字节) [346:349]
        if (length < offset + 4) return false;
        visibleLightDataLength = readInt32(data, offset);
        offset += 4;
        
        // 15. 红外照片数据长度 (4 字节) [350:353]
        if (length < offset + 4) return false;
        infraredPhotoDataLength = readInt32(data, offset);
        offset += 4;
        
        // 16. 辐射率 (4 字节) [354:357]
        if (length < offset + 4) return false;
        emissivity = readFloat(data, offset);
        offset += 4;
        
        // 17. 测试距离 (4 字节) [358:361]
        if (length < offset + 4) return false;
        testDistance = readFloat(data, offset);
        offset += 4;
        
        // 18. 大气温度 (4 字节) [362:365]
        if (length < offset + 4) return false;
        atmosphericTemperature = readFloat(data, offset);
        offset += 4;
        
        // 19. 相对湿度 (1 字节) [366:366]
        if (length < offset + 1) return false;
        relativeHumidity = data[offset];
        offset += 1;
        
        // 20. 反射温度 (4 字节) [367:370]
        if (length < offset + 4) return false;
        reflectedTemperature = readFloat(data, offset);
        offset += 4;
        
        // 21. 温宽上限 (4 字节) [371:374]
        if (length < offset + 4) return false;
        temperatureRangeUpper = readFloat(data, offset);
        offset += 4;
        
        // 22. 温宽下限 (4 字节) [375:378]
        if (length < offset + 4) return false;
        temperatureRangeLower = readFloat(data, offset);
        offset += 4;
        
        // 23. 可见光照片数据 (L1 字节) [379:379+L1-1]
        if (visibleLightDataLength > 0)
        {
            if (length < offset + visibleLightDataLength) return false;
            visibleLightPhotoData.assign(data + offset, data + offset + visibleLightDataLength);
            offset += visibleLightDataLength;
        }
        
        // 24. 红外照片数据 (L2 字节) [379+L1:379+L1+L2-1]
        if (infraredPhotoDataLength > 0)
        {
            if (length < offset + infraredPhotoDataLength) return false;
            infraredPhotoData.assign(data + offset, data + offset + infraredPhotoDataLength);
            offset += infraredPhotoDataLength;
        }
        
        return true;
    }
    
    /**
     * @brief 从 vector 字节流解析红外图谱数据
     */
    bool parseFromBytes(const std::vector<uint8_t>& data)
    {
        return parseFromBytes(data.data(), data.size());
    }
    
    /**
     * @brief 将解析后的数据转换为字节流（用于序列化）
     * @return 完整的字节流数据
     */
    std::vector<uint8_t> toBytes() const
    {
        std::vector<uint8_t> buffer;
        
        // 1. 检测数据类型编码 (1 字节)
        buffer.push_back(dataTypeCode);
        
        // 2. 图谱数据长度 (4 字节)
        writeInt32(buffer, dataLength);
        
        // 3. 图谱生成时间 (8 字节)
        writeInt64(buffer, createTime);
        
        // 4. 图谱性质 (1 字节)
        buffer.push_back(spectrumProperty);
        
        // 5. 被检测设备名称 (118 字节)
        for (size_t i = 0; i < 118; i++)
        {
            buffer.push_back(i < deviceName.size() ? deviceName[i] : 0);
        }
        
        // 6. 被检测设备编码 (42 字节)
        for (size_t i = 0; i < 42; i++)
        {
            buffer.push_back(i < deviceCode.size() ? deviceCode[i] : 0);
        }
        
        // 7. 测点名称 (128 字节)
        for (size_t i = 0; i < 128; i++)
        {
            buffer.push_back(i < measurementPointName.size() ? measurementPointName[i] : 0);
        }
        
        // 8. 测点编码 (32 字节)
        for (size_t i = 0; i < 32; i++)
        {
            buffer.push_back(i < measurementPointCode.size() ? measurementPointCode[i] : 0);
        }
        
        // 9. 检测通道标志 (2 字节)
        writeInt16(buffer, detectionChannelFlag);
        
        // 10. 存储器数据类型 (1 字节)
        buffer.push_back(storageDataType);
        
        // 11. 温度单位 (1 字节)
        buffer.push_back(temperatureUnit);
        
        // 12. 温度点阵宽度 (4 字节)
        writeInt32(buffer, temperatureMatrixWidth);
        
        // 13. 温度点阵高度 (4 字节)
        writeInt32(buffer, temperatureMatrixHeight);
        
        // 14. 可见光照片数据长度 (4 字节)
        writeInt32(buffer, visibleLightDataLength);
        
        // 15. 红外照片数据长度 (4 字节)
        writeInt32(buffer, infraredPhotoDataLength);
        
        // 16. 辐射率 (4 字节)
        writeFloat(buffer, emissivity);
        
        // 17. 测试距离 (4 字节)
        writeFloat(buffer, testDistance);
        
        // 18. 大气温度 (4 字节)
        writeFloat(buffer, atmosphericTemperature);
        
        // 19. 相对湿度 (1 字节)
        buffer.push_back(relativeHumidity);
        
        // 20. 反射温度 (4 字节)
        writeFloat(buffer, reflectedTemperature);
        
        // 21. 温宽上限 (4 字节)
        writeFloat(buffer, temperatureRangeUpper);
        
        // 22. 温宽下限 (4 字节)
        writeFloat(buffer, temperatureRangeLower);
        
        // 23. 可见光照片数据 (L1 字节)
        for (size_t i = 0; i < visibleLightPhotoData.size(); i++)
        {
            buffer.push_back(visibleLightPhotoData[i]);
        }
        
        // 24. 红外照片数据 (L2 字节)
        for (size_t i = 0; i < infraredPhotoData.size(); i++)
        {
            buffer.push_back(infraredPhotoData[i]);
        }
        
        return buffer;
    }
    
    // ==================== 辅助方法 ====================
    
    /**
     * @brief 获取设备名称字符串（UNICODE 解码）
     */
    std::string getDeviceNameString() const
    {
        return decodeUnicodeString(deviceName);
    }
    
    /**
     * @brief 获取设备编码字符串（ASCII 解码）
     */
    std::string getDeviceCodeString() const
    {
        return decodeAsciiString(deviceCode);
    }
    
    /**
     * @brief 获取测点名称字符串（UNICODE 解码）
     */
    std::string getMeasurementPointNameString() const
    {
        return decodeUnicodeString(measurementPointName);
    }
    
    /**
     * @brief 获取测点编码字符串（ASCII 解码）
     */
    std::string getMeasurementPointCodeString() const
    {
        return decodeAsciiString(measurementPointCode);
    }
    
    /**
     * @brief 获取图谱生成时间字符串（格式：YYYY-MM-DD HH:mm:ss.fff）
     */
    std::string getCreateTimeString() const
    {
        // int64 格式：YYYYMMDDhhmmssfff
        // 例如：20100818151010001
        char buffer[64];
        snprintf(buffer, sizeof(buffer), 
                 "%04lld-%02lld-%02lld %02lld:%02lld:%02lld.%03lld",
                 (long long)(createTime / 10000000000000LL) % 10000LL,  // 年
                 (long long)(createTime / 100000000000LL) % 100LL,      // 月
                 (long long)(createTime / 1000000000LL) % 100LL,        // 日
                 (long long)(createTime / 10000000LL) % 100LL,          // 时
                 (long long)(createTime / 100000LL) % 100LL,            // 分
                 (long long)(createTime / 1000LL) % 100LL,              // 秒
                 (long long)createTime % 1000LL);                       // 毫秒
        return std::string(buffer);
    }
    
    /**
     * @brief 获取图谱性质描述
     */
    std::string getSpectrumPropertyDescription() const
    {
        switch (spectrumProperty)
        {
            case 0x03: return "检测设备单相整体图谱";
            case 0x04: return "检测设备注意相局部图谱";
            case 0x05: return "检测设备所属三相设备图谱";
            default: return "未知图谱性质";
        }
    }
    
private:
    // ==================== 辅助读写函数（大端模式）====================
    
    static int16_t readInt16(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<int16_t>(data[offset]) << 8) | 
               static_cast<int16_t>(data[offset + 1]);
    }
    
    static int32_t readInt32(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<int32_t>(data[offset]) << 24) | 
               (static_cast<int32_t>(data[offset + 1]) << 16) | 
               (static_cast<int32_t>(data[offset + 2]) << 8) | 
               static_cast<int32_t>(data[offset + 3]);
    }
    
    static int64_t readInt64(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        return (static_cast<int64_t>(data[offset]) << 56) | 
               (static_cast<int64_t>(data[offset + 1]) << 48) | 
               (static_cast<int64_t>(data[offset + 2]) << 40) | 
               (static_cast<int64_t>(data[offset + 3]) << 32) | 
               (static_cast<int64_t>(data[offset + 4]) << 24) | 
               (static_cast<int64_t>(data[offset + 5]) << 16) | 
               (static_cast<int64_t>(data[offset + 6]) << 8) | 
               static_cast<int64_t>(data[offset + 7]);
    }
    
    static float readFloat(const uint8_t* data, size_t offset)
    {
        // 大端模式：高位字节在前
        uint32_t intVal = (static_cast<uint32_t>(data[offset]) << 24) | 
                         (static_cast<uint32_t>(data[offset + 1]) << 16) | 
                         (static_cast<uint32_t>(data[offset + 2]) << 8) | 
                         static_cast<uint32_t>(data[offset + 3]);
        
        // 将 int32 转换为 float
        float result;
        std::memcpy(&result, &intVal, sizeof(float));
        return result;
    }
    
    static void writeInt16(std::vector<uint8_t>& buffer, int16_t value)
    {
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeInt32(std::vector<uint8_t>& buffer, int32_t value)
    {
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeInt64(std::vector<uint8_t>& buffer, int64_t value)
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
    
    static void writeFloat(std::vector<uint8_t>& buffer, float value)
    {
        // 将 float 转换为 uint32
        uint32_t intVal;
        std::memcpy(&intVal, &value, sizeof(float));
        
        // 大端模式：高位字节在前
        buffer.push_back(static_cast<uint8_t>((intVal >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(intVal & 0xFF));
    }
    
    /**
     * @brief 解码 UNICODE 字符串（UTF-16 BE）
     */
    std::string decodeUnicodeString(const std::vector<uint8_t>& data) const
    {
        std::string result;
        for (size_t i = 0; i + 1 < data.size(); i += 2)
        {
            if (data[i] == 0 && data[i + 1] == 0)
            {
                break; // 遇到结束符
            }
            
            // 大端模式：高位字节在前
            wchar_t ch = (static_cast<wchar_t>(data[i]) << 8) | 
                        static_cast<wchar_t>(data[i + 1]);
            
            if (ch < 128)
            {
                result += static_cast<char>(ch);
            }
            else
            {
                // 简单处理：非 ASCII 字符用?代替
                result += '?';
            }
        }
        return result;
    }
    
    /**
     * @brief 解码 ASCII 字符串
     */
    std::string decodeAsciiString(const std::vector<uint8_t>& data) const
    {
        std::string result;
        for (size_t i = 0; i < data.size(); i++)
        {
            if (data[i] == 0)
            {
                break; // 遇到结束符
            }
            result += static_cast<char>(data[i]);
        }
        return result;
    }
};
