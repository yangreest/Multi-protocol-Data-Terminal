#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

/**
 * @brief 图像数据格式解析类
 * 根据文件数据格式规范（表 2）定义
 * 
 * 数据格式说明:
 * - 所有多字节整数类型均采用大端模式（Big-Endian）
 * - 字符串采用 UNICODE 编码或 ASCII 编码
 * - 字节顺序严格按照定义顺序排列
 */
class ImageData
{
public:
    // ==================== 文件头部字段 ====================
    
    // 1. 文件长度 L (int32, 4 字节) [0:3] - 必备
    int32_t fileLength;
    
    // 2. 规范版本号 (uint8, 4 字节) [4:7] - 必备
    std::vector<uint8_t> versionNumber;
    
    // 3. 文件生成时间 (int64, 8 字节) [8:15] - 必备
    int64_t createTime;
    
    // 4. 站点名称 (char, 118 字节) [16:133] - 可选
    std::vector<uint8_t> stationName;
    
    // 5. 站点编码 (char, 42 字节) [134:175] - 可选
    // ASCII 编码，以\0 结尾
    std::vector<uint8_t> stationCode;
    
    // 6. 天气 (uint8, 1 字节) [176:176] - 可选
    // 0xFF:未记录，0x01:晴，0x02:阴，0x03:雨，0x04:雪
    // 0x05:雾，0x06:雷雨，0x07:多云
    uint8_t weather;
    
    // 7. 温度 (float, 4 字节) [177:180] - 可选
    // 环境温度，单位摄氏度
    float temperature;
    
    // 8. 湿度 (int8, 1 字节) [181:181] - 可选
    // 环境湿度，单位%
    int8_t humidity;
    
    // 9. 仪器厂家 (char, 32 字节) [182:213] - 必备
    // UNICODE 编码，以 0x0000 结尾
    std::vector<uint8_t> instrumentManufacturer;
    
    // 10. 仪器型号 (char, 32 字节) [214:245] - 必备
    // UNICODE 编码，以 0x0000 结尾
    std::vector<uint8_t> instrumentModel;
    
    // 11. 仪器版本号 (uint8, 4 字节) [246:249] - 可选
    std::vector<uint8_t> instrumentVersion;
    
    // 12. 仪器序列号 (char, 32 字节) [250:281] - 必备
    // ASCII 编码，以\0 结尾
    std::vector<uint8_t> instrumentSerialNumber;
    
    // 13. 系统频率 (float, 4 字节) [282:285] - 必备
    // 单位 Hz，例如 50Hz
    float systemFrequency;
    
    // 14. 图谱数量 N (int16, 2 字节) [286:287] - 必备
    int16_t spectrumCount;
    
    // 15. 经度 (double, 8 字节) [288:295] - 可选
    // 0 代表不支持该参数
    double longitude;
    
    // 16. 纬度 (double, 8 字节) [296:303] - 可选
    // 0 代表不支持该参数
    double latitude;
    
    // 17. 海拔 (int32, 4 字节) [304:307] - 可选
    // 单位 m，0 代表不支持该参数
    int32_t altitude;
    
    // 18. 预留 (自定义，204 字节) [308:511] - 可选
    std::vector<uint8_t> reserved;
    
    // 19. 图谱数据 (动态长度) - 依次存放同一检测方法的多类型数据
    std::vector<std::vector<uint8_t>> spectrumData;
    
    // 20. 文件尾部预留 (32 字节) [L-36:L-5] - 必备
    std::vector<uint8_t> tailReserved;
    
    // 21. CRC32 (int32, 4 字节) [L-4:L-1] - 必备
    int32_t crc32;
    
    // ==================== 构造函数 ====================
    
    ImageData()
    {
        fileLength = 0;
        versionNumber = {0x01, 0x00, 0x00, 0x00};
        createTime = 0;
        stationName.resize(118, 0);
        stationCode.resize(42, 0);
        weather = 0xFF;
        temperature = 0.0f;
        humidity = 0;
        instrumentManufacturer.resize(32, 0);
        instrumentModel.resize(32, 0);
        instrumentVersion = {0x01, 0x00, 0x00, 0x00};
        instrumentSerialNumber.resize(32, 0);
        systemFrequency = 50.0f;
        spectrumCount = 0;
        longitude = 0.0;
        latitude = 0.0;
        altitude = 0;
        reserved.resize(204, 0);
        tailReserved.resize(32, 0);
        crc32 = 0;
    }
    
    // ==================== 解析方法 ====================
    
    /**
     * @brief 从字节流解析图像数据
     * @param data 字节流数据
     * @return 解析是否成功
     */
    bool parseFromBytes(const std::vector<uint8_t>& data)
    {
        // 最小长度检查：文件头部 512 字节 + 文件尾部 36 字节 = 548 字节
        if (data.size() < 548)
        {
            return false;
        }
        
        size_t offset = 0;
        
        // 1. 解析文件长度 (4 字节)
        fileLength = readInt32(data.data(), offset);
        offset += 4;
        
        // 2. 解析规范版本号 (4 字节)
        versionNumber.clear();
        for (int i = 0; i < 4; i++)
        {
            versionNumber.push_back(data[offset + i]);
        }
        offset += 4;
        
        // 3. 解析文件生成时间 (8 字节)
        createTime = readInt64(data.data(), offset);
        offset += 8;
        
        // 4. 解析站点名称 (118 字节)
        stationName.clear();
        for (int i = 0; i < 118; i++)
        {
            stationName.push_back(data[offset + i]);
        }
        offset += 118;
        
        // 5. 解析站点编码 (42 字节)
        stationCode.clear();
        for (int i = 0; i < 42; i++)
        {
            stationCode.push_back(data[offset + i]);
        }
        offset += 42;
        
        // 6. 解析天气 (1 字节)
        weather = data[offset];
        offset += 1;
        
        // 7. 解析温度 (4 字节)
        temperature = readFloat(data.data(), offset);
        offset += 4;
        
        // 8. 解析湿度 (1 字节)
        humidity = static_cast<int8_t>(data[offset]);
        offset += 1;
        
        // 9. 解析仪器厂家 (32 字节)
        instrumentManufacturer.clear();
        for (int i = 0; i < 32; i++)
        {
            instrumentManufacturer.push_back(data[offset + i]);
        }
        offset += 32;
        
        // 10. 解析仪器型号 (32 字节)
        instrumentModel.clear();
        for (int i = 0; i < 32; i++)
        {
            instrumentModel.push_back(data[offset + i]);
        }
        offset += 32;
        
        // 11. 解析仪器版本号 (4 字节)
        instrumentVersion.clear();
        for (int i = 0; i < 4; i++)
        {
            instrumentVersion.push_back(data[offset + i]);
        }
        offset += 4;
        
        // 12. 解析仪器序列号 (32 字节)
        instrumentSerialNumber.clear();
        for (int i = 0; i < 32; i++)
        {
            instrumentSerialNumber.push_back(data[offset + i]);
        }
        offset += 32;
        
        // 13. 解析系统频率 (4 字节)
        systemFrequency = readFloat(data.data(), offset);
        offset += 4;
        
        // 14. 解析图谱数量 (2 字节)
        spectrumCount = readInt16(data.data(), offset);
        offset += 2;
        
        // 15. 解析经度 (8 字节)
        longitude = readDouble(data.data(), offset);
        offset += 8;
        
        // 16. 解析纬度 (8 字节)
        latitude = readDouble(data.data(), offset);
        offset += 8;
        
        // 17. 解析海拔 (4 字节)
        altitude = readInt32(data.data(), offset);
        offset += 4;
        
        // 18. 解析预留字段 (204 字节)
        reserved.clear();
        for (int i = 0; i < 204; i++)
        {
            reserved.push_back(data[offset + i]);
        }
        offset += 204;
        
        // 此时 offset = 512，接下来是图谱数据区域
        // 文件尾部固定 36 字节（32 字节预留 + 4 字节 CRC32）
        // 图谱数据长度 = fileLength - 512 - 36
        size_t spectrumDataLength = fileLength - 512 - 36;
        
        if (data.size() < offset + spectrumDataLength + 36)
        {
            return false;
        }
        
        // 19. 解析图谱数据（根据具体格式解析，这里先按原始字节存储）
        if (spectrumDataLength > 0)
        {
            std::vector<uint8_t> spectrumBytes;
            for (size_t i = 0; i < spectrumDataLength; i++)
            {
                spectrumBytes.push_back(data[offset + i]);
            }
            spectrumData.clear();
            spectrumData.push_back(spectrumBytes);
            offset += spectrumDataLength;
        }
        
        // 20. 解析文件尾部预留 (32 字节)
        tailReserved.clear();
        for (int i = 0; i < 32; i++)
        {
            tailReserved.push_back(data[offset + i]);
        }
        offset += 32;
        
        // 21. 解析 CRC32 (4 字节)
        crc32 = readInt32(data.data(), offset);
        offset += 4;
        
        return true;
    }
    
    /**
     * @brief 从字节流解析图像数据（原始指针版本）
     */
    bool parseFromBytes(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < 548)
        {
            return false;
        }
        
        std::vector<uint8_t> vec(data, data + size);
        return parseFromBytes(vec);
    }
    
    // ==================== 序列化方法 ====================
    
    /**
     * @brief 将图像数据序列化为字节流
     * @return 序列化后的字节流
     */
    std::vector<uint8_t> toBytes() const
    {
        std::vector<uint8_t> buffer;
        
        // 计算总长度：512(头部) + 图谱数据长度 + 36(尾部)
        size_t spectrumDataLength = 0;
        for (const auto& spectrum : spectrumData)
        {
            spectrumDataLength += spectrum.size();
        }
        size_t totalLength = 512 + spectrumDataLength + 36;
        
        buffer.reserve(totalLength);
        
        // 1. 文件长度 (4 字节)
        writeInt32(buffer, fileLength);
        
        // 2. 规范版本号 (4 字节)
        for (size_t i = 0; i < 4; i++)
        {
            buffer.push_back(i < versionNumber.size() ? versionNumber[i] : 0);
        }
        
        // 3. 文件生成时间 (8 字节)
        writeInt64(buffer, createTime);
        
        // 4. 站点名称 (118 字节)
        for (size_t i = 0; i < 118; i++)
        {
            buffer.push_back(i < stationName.size() ? stationName[i] : 0);
        }
        
        // 5. 站点编码 (42 字节)
        for (size_t i = 0; i < 42; i++)
        {
            buffer.push_back(i < stationCode.size() ? stationCode[i] : 0);
        }
        
        // 6. 天气 (1 字节)
        buffer.push_back(weather);
        
        // 7. 温度 (4 字节)
        writeFloat(buffer, temperature);
        
        // 8. 湿度 (1 字节)
        buffer.push_back(static_cast<uint8_t>(humidity));
        
        // 9. 仪器厂家 (32 字节)
        for (size_t i = 0; i < 32; i++)
        {
            buffer.push_back(i < instrumentManufacturer.size() ? instrumentManufacturer[i] : 0);
        }
        
        // 10. 仪器型号 (32 字节)
        for (size_t i = 0; i < 32; i++)
        {
            buffer.push_back(i < instrumentModel.size() ? instrumentModel[i] : 0);
        }
        
        // 11. 仪器版本号 (4 字节)
        for (size_t i = 0; i < 4; i++)
        {
            buffer.push_back(i < instrumentVersion.size() ? instrumentVersion[i] : 0);
        }
        
        // 12. 仪器序列号 (32 字节)
        for (size_t i = 0; i < 32; i++)
        {
            buffer.push_back(i < instrumentSerialNumber.size() ? instrumentSerialNumber[i] : 0);
        }
        
        // 13. 系统频率 (4 字节)
        writeFloat(buffer, systemFrequency);
        
        // 14. 图谱数量 (2 字节)
        writeInt16(buffer, spectrumCount);
        
        // 15. 经度 (8 字节)
        writeDouble(buffer, longitude);
        
        // 16. 纬度 (8 字节)
        writeDouble(buffer, latitude);
        
        // 17. 海拔 (4 字节)
        writeInt32(buffer, altitude);
        
        // 18. 预留 (204 字节)
        for (size_t i = 0; i < 204; i++)
        {
            buffer.push_back(i < reserved.size() ? reserved[i] : 0);
        }
        
        // 19. 图谱数据
        for (const auto& spectrum : spectrumData)
        {
            for (size_t i = 0; i < spectrum.size(); i++)
            {
                buffer.push_back(spectrum[i]);
            }
        }
        
        // 20. 文件尾部预留 (32 字节)
        for (size_t i = 0; i < 32; i++)
        {
            buffer.push_back(i < tailReserved.size() ? tailReserved[i] : 0);
        }
        
        // 21. CRC32 (4 字节)
        writeInt32(buffer, crc32);
        
        return buffer;
    }
    
    // ==================== 辅助方法 ====================
    
    /**
     * @brief 获取规范版本号字符串（格式：X.X.X.X）
     */
    std::string getVersionString() const
    {
        if (versionNumber.size() < 4)
        {
            return "0.0.0.0";
        }
        
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
                 versionNumber[0], versionNumber[1], 
                 versionNumber[2], versionNumber[3]);
        return std::string(buffer);
    }
    
    /**
     * @brief 获取文件生成时间字符串（格式：YYYY-MM-DD HH:mm:ss.fff）
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
     * @brief 获取站点名称字符串（UNICODE 解码）
     */
    std::string getStationNameString() const
    {
        return decodeUnicodeString(stationName);
    }
    
    /**
     * @brief 获取站点编码字符串（ASCII 解码）
     */
    std::string getStationCodeString() const
    {
        return decodeAsciiString(stationCode);
    }
    
    /**
     * @brief 获取天气描述
     */
    std::string getWeatherDescription() const
    {
        switch (weather)
        {
            case 0xFF: return "未记录";
            case 0x01: return "晴";
            case 0x02: return "阴";
            case 0x03: return "雨";
            case 0x04: return "雪";
            case 0x05: return "雾";
            case 0x06: return "雷雨";
            case 0x07: return "多云";
            default: return "未知";
        }
    }
    
    /**
     * @brief 获取仪器厂家字符串（UNICODE 解码）
     */
    std::string getInstrumentManufacturerString() const
    {
        return decodeUnicodeString(instrumentManufacturer);
    }
    
    /**
     * @brief 获取仪器型号字符串（UNICODE 解码）
     */
    std::string getInstrumentModelString() const
    {
        return decodeUnicodeString(instrumentModel);
    }
    
    /**
     * @brief 获取仪器版本号字符串（格式：X.X.X.X）
     */
    std::string getInstrumentVersionString() const
    {
        if (instrumentVersion.size() < 4)
        {
            return "0.0.0.0";
        }
        
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
                 instrumentVersion[0], instrumentVersion[1], 
                 instrumentVersion[2], instrumentVersion[3]);
        return std::string(buffer);
    }
    
    /**
     * @brief 获取仪器序列号字符串（ASCII 解码）
     */
    std::string getInstrumentSerialNumberString() const
    {
        return decodeAsciiString(instrumentSerialNumber);
    }
    
    /**
     * @brief 获取地理位置信息
     */
    std::string getLocationString() const
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "经度：%.6f°, 纬度：%.6f°, 海拔：%dm",
                 longitude, latitude, altitude);
        return std::string(buffer);
    }
    
    /**
     * @brief 设置站点名称（字符串转 UNICODE 编码）
     */
    void setStationName(const std::string& name)
    {
        stationName = encodeUnicodeString(name, 118);
    }
    
    /**
     * @brief 设置站点编码（字符串转 ASCII 编码）
     */
    void setStationCode(const std::string& code)
    {
        stationCode = encodeAsciiString(code, 42);
    }
    
    /**
     * @brief 设置仪器厂家（字符串转 UNICODE 编码）
     */
    void setInstrumentManufacturer(const std::string& name)
    {
        instrumentManufacturer = encodeUnicodeString(name, 32);
    }
    
    /**
     * @brief 设置仪器型号（字符串转 UNICODE 编码）
     */
    void setInstrumentModel(const std::string& model)
    {
        instrumentModel = encodeUnicodeString(model, 32);
    }
    
    /**
     * @brief 设置仪器序列号（字符串转 ASCII 编码）
     */
    void setInstrumentSerialNumber(const std::string& serial)
    {
        instrumentSerialNumber = encodeAsciiString(serial, 32);
    }
    
    /**
     * @brief 设置规范版本号（从字符串解析）
     */
    bool setVersionFromString(const std::string& version)
    {
        unsigned int v0, v1, v2, v3;
        if (sscanf_s(version.c_str(), "%u.%u.%u.%u", &v0, &v1, &v2, &v3) == 4)
        {
            versionNumber = {
                static_cast<uint8_t>(v0),
                static_cast<uint8_t>(v1),
                static_cast<uint8_t>(v2),
                static_cast<uint8_t>(v3)
            };
            return true;
        }
        return false;
    }
    
    /**
     * @brief 设置仪器版本号（从字符串解析）
     */
    bool setInstrumentVersionFromString(const std::string& version)
    {
        unsigned int v0, v1, v2, v3;
        if (sscanf_s(version.c_str(), "%u.%u.%u.%u", &v0, &v1, &v2, &v3) == 4)
        {
            instrumentVersion = {
                static_cast<uint8_t>(v0),
                static_cast<uint8_t>(v1),
                static_cast<uint8_t>(v2),
                static_cast<uint8_t>(v3)
            };
            return true;
        }
        return false;
    }
    
    /**
     * @brief 设置文件生成时间（从字符串解析）
     * @param timeStr 时间字符串（格式：YYYY-MM-DD HH:mm:ss.fff）
     * @return 设置是否成功
     */
    bool setCreateTimeFromString(const std::string& timeStr)
    {
        unsigned int year, month, day, hour, minute, second, millisecond;
        if (sscanf_s(timeStr.c_str(), "%u-%u-%u %u:%u:%u.%u",
                     &year, &month, &day, &hour, &minute, &second, &millisecond) == 7)
        {
            createTime = (int64_t)year * 10000000000000LL +
                        (int64_t)month * 100000000000LL +
                        (int64_t)day * 1000000000LL +
                        (int64_t)hour * 10000000LL +
                        (int64_t)minute * 100000LL +
                        (int64_t)second * 1000LL +
                        (int64_t)millisecond;
            return true;
        }
        return false;
    }
    
private:
    // ==================== 辅助读写函数（大端模式）====================
    
    static int16_t readInt16(const uint8_t* data, size_t offset)
    {
        return (static_cast<int16_t>(data[offset]) << 8) | 
               static_cast<int16_t>(data[offset + 1]);
    }
    
    static int32_t readInt32(const uint8_t* data, size_t offset)
    {
        return (static_cast<int32_t>(data[offset]) << 24) | 
               (static_cast<int32_t>(data[offset + 1]) << 16) | 
               (static_cast<int32_t>(data[offset + 2]) << 8) | 
               static_cast<int32_t>(data[offset + 3]);
    }
    
    static int64_t readInt64(const uint8_t* data, size_t offset)
    {
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
        uint32_t intVal = (static_cast<uint32_t>(data[offset]) << 24) | 
                         (static_cast<uint32_t>(data[offset + 1]) << 16) | 
                         (static_cast<uint32_t>(data[offset + 2]) << 8) | 
                         static_cast<uint32_t>(data[offset + 3]);
        
        float result;
        std::memcpy(&result, &intVal, sizeof(float));
        return result;
    }
    
    static double readDouble(const uint8_t* data, size_t offset)
    {
        uint64_t intVal = (static_cast<uint64_t>(data[offset]) << 56) | 
                         (static_cast<uint64_t>(data[offset + 1]) << 48) | 
                         (static_cast<uint64_t>(data[offset + 2]) << 40) | 
                         (static_cast<uint64_t>(data[offset + 3]) << 32) | 
                         (static_cast<uint64_t>(data[offset + 4]) << 24) | 
                         (static_cast<uint64_t>(data[offset + 5]) << 16) | 
                         (static_cast<uint64_t>(data[offset + 6]) << 8) | 
                         static_cast<uint64_t>(data[offset + 7]);
        
        double result;
        std::memcpy(&result, &intVal, sizeof(double));
        return result;
    }
    
    static void writeInt16(std::vector<uint8_t>& buffer, int16_t value)
    {
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeInt32(std::vector<uint8_t>& buffer, int32_t value)
    {
        buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    
    static void writeInt64(std::vector<uint8_t>& buffer, int64_t value)
    {
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
        uint32_t intVal;
        std::memcpy(&intVal, &value, sizeof(float));
        
        buffer.push_back(static_cast<uint8_t>((intVal >> 24) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 16) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 8) & 0xFF));
        buffer.push_back(static_cast<uint8_t>(intVal & 0xFF));
    }
    
    static void writeDouble(std::vector<uint8_t>& buffer, double value)
    {
        uint64_t intVal;
        std::memcpy(&intVal, &value, sizeof(double));
        
        buffer.push_back(static_cast<uint8_t>((intVal >> 56) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 48) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 40) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((intVal >> 32) & 0xFF));
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
                // 简单处理：非 ASCII 字符用？代替
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
                break;
            }
            result += static_cast<char>(data[i]);
        }
        return result;
    }
    
    /**
     * @brief 编码字符串为 UNICODE（UTF-16 BE）
     */
    std::vector<uint8_t> encodeUnicodeString(const std::string& str, size_t fixedLength) const
    {
        std::vector<uint8_t> result;
        
        for (size_t i = 0; i < str.length(); i++)
        {
            char ch = str[i];
            result.push_back(static_cast<uint8_t>((ch >> 8) & 0xFF));
            result.push_back(static_cast<uint8_t>(ch & 0xFF));
        }
        
        result.push_back(0x00);
        result.push_back(0x00);
        
        while (result.size() < fixedLength)
        {
            result.push_back(0x00);
        }
        
        if (result.size() > fixedLength)
        {
            result.resize(fixedLength);
        }
        
        return result;
    }
    
    /**
     * @brief 编码字符串为 ASCII
     */
    std::vector<uint8_t> encodeAsciiString(const std::string& str, size_t fixedLength) const
    {
        std::vector<uint8_t> result;
        
        for (size_t i = 0; i < str.length(); i++)
        {
            result.push_back(static_cast<uint8_t>(str[i]));
        }
        
        result.push_back(0x00);
        
        while (result.size() < fixedLength)
        {
            result.push_back(0x00);
        }
        
        if (result.size() > fixedLength)
        {
            result.resize(fixedLength);
        }
        
        return result;
    }
};
