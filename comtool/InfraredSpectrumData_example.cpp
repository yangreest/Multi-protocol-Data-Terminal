/**
 * @brief 使用示例：解析 detectionFileData 到 InfraredSpectrumData
 * 
 * 示例代码展示了如何使用 InfraredSpectrumData 类解析通信协议中的检测数据文件
 */

#include "InfraredSpectrumData.h"
#include "CommunicationProtocol.h"
#include <iostream>

/**
 * @brief 示例 1：从 CommunicationProtocol 中解析红外图谱数据
 */
void example1_ParseFromCommunicationProtocol()
{
    // 假设已经有一个 CommunicationProtocol 对象
    CommunicationProtocol protocol;
    
    // ... 这里通过某种方式填充了 protocol.detectionFileData ...
    // 例如：从网络接收、从文件读取等
    
    // 创建红外图谱数据对象
    InfraredSpectrumData spectrumData;
    
    // 解析 detectionFileData
    if (spectrumData.parseFromBytes(protocol.detectionFileData))
    {
        // 解析成功，访问各个字段
        
        // 1. 基本信息
        std::cout << "数据类型编码：" << (int)spectrumData.dataTypeCode << std::endl;
        std::cout << "图谱数据长度：" << spectrumData.dataLength << std::endl;
        std::cout << "生成时间：" << spectrumData.getCreateTimeString() << std::endl;
        std::cout << "图谱性质：" << spectrumData.getSpectrumPropertyDescription() << std::endl;
        
        // 2. 设备信息
        std::cout << "设备名称：" << spectrumData.getDeviceNameString() << std::endl;
        std::cout << "设备编码：" << spectrumData.getDeviceCodeString() << std::endl;
        std::cout << "测点名称：" << spectrumData.getMeasurementPointNameString() << std::endl;
        std::cout << "测点编码：" << spectrumData.getMeasurementPointCodeString() << std::endl;
        
        // 3. 检测参数
        std::cout << "检测通道标志：" << spectrumData.detectionChannelFlag << std::endl;
        std::cout << "存储器数据类型：" << (int)spectrumData.storageDataType << std::endl;
        std::cout << "温度单位：" << (int)spectrumData.temperatureUnit << std::endl;
        
        // 4. 图像参数
        std::cout << "温度矩阵宽度：" << spectrumData.temperatureMatrixWidth << std::endl;
        std::cout << "温度矩阵高度：" << spectrumData.temperatureMatrixHeight << std::endl;
        
        // 5. 照片数据长度
        std::cout << "可见光照片长度：" << spectrumData.visibleLightDataLength << std::endl;
        std::cout << "红外照片长度：" << spectrumData.infraredPhotoDataLength << std::endl;
        
        // 6. 环境参数
        std::cout << "辐射率：" << spectrumData.emissivity << std::endl;
        std::cout << "测试距离：" << spectrumData.testDistance << " m" << std::endl;
        std::cout << "大气温度：" << spectrumData.atmosphericTemperature << " °C" << std::endl;
        std::cout << "相对湿度：" << (int)spectrumData.relativeHumidity << " %" << std::endl;
        std::cout << "反射温度：" << spectrumData.reflectedTemperature << " °C" << std::endl;
        
        // 7. 温宽参数
        std::cout << "温宽上限：" << spectrumData.temperatureRangeUpper << std::endl;
        std::cout << "温宽下限：" << spectrumData.temperatureRangeLower << std::endl;
        
        // 8. 访问照片数据
        if (!spectrumData.visibleLightPhotoData.empty())
        {
            std::cout << "可见光照片数据大小：" << spectrumData.visibleLightPhotoData.size() << " 字节" << std::endl;
            // 可以将 visibleLightPhotoData 保存到文件或显示
        }
        
        if (!spectrumData.infraredPhotoData.empty())
        {
            std::cout << "红外照片数据大小：" << spectrumData.infraredPhotoData.size() << " 字节" << std::endl;
            // 可以将 infraredPhotoData 保存到文件或显示
        }
    }
    else
    {
        std::cerr << "解析红外图谱数据失败！" << std::endl;
    }
}

/**
 * @brief 示例 2：将红外图谱数据序列化为字节流
 */
void example2_SerializeToBytes()
{
    // 创建并填充红外图谱数据对象
    InfraredSpectrumData spectrumData;
    
    // 填充基本信息
    spectrumData.dataTypeCode = 0x01;
    spectrumData.dataLength = 1024;
    spectrumData.createTime = 20240401153000123LL; // 2024-04-01 15:30:00.123
    spectrumData.spectrumProperty = 0x03; // 单相整体图谱
    
    // 填充设备信息（UNICODE 编码）
    std::string deviceName = "测试设备";
    // 注意：实际使用时需要将字符串转换为 UNICODE 编码
    spectrumData.deviceName.resize(118, 0);
    
    // 填充测点信息
    std::string pointName = "测温点 1";
    spectrumData.measurementPointName.resize(128, 0);
    
    // 填充检测参数
    spectrumData.detectionChannelFlag = 1;
    spectrumData.storageDataType = 0x01;
    spectrumData.temperatureUnit = 0x01; // 摄氏度
    
    // 填充图像参数
    spectrumData.temperatureMatrixWidth = 320;
    spectrumData.temperatureMatrixHeight = 240;
    
    // 填充环境参数
    spectrumData.emissivity = 0.95f;
    spectrumData.testDistance = 10.5f;
    spectrumData.atmosphericTemperature = 25.0f;
    spectrumData.relativeHumidity = 60;
    spectrumData.reflectedTemperature = 20.0f;
    
    // 填充温宽参数
    spectrumData.temperatureRangeUpper = 100.0f;
    spectrumData.temperatureRangeLower = 0.0f;
    
    // 序列化为字节流
    std::vector<uint8_t> bytes = spectrumData.toBytes();
    
    std::cout << "序列化后的字节流长度：" << bytes.size() << std::endl;
    
    // 可以将字节流设置到 CommunicationProtocol 中
    CommunicationProtocol protocol;
    protocol.setDetectionFile(bytes);
    
    // 然后可以通过 protocol.toBytes() 生成完整的通信报文
}

/**
 * @brief 示例 3：从字节流直接解析（不通过 CommunicationProtocol）
 */
void example3_DirectParseFromBytes()
{
    // 假设有原始的字节流数据
    std::vector<uint8_t> rawData;
    // ... 填充 rawData ...
    
    // 直接解析
    InfraredSpectrumData spectrumData;
    if (spectrumData.parseFromBytes(rawData))
    {
        std::cout << "解析成功！" << std::endl;
        std::cout << "设备名称：" << spectrumData.getDeviceNameString() << std::endl;
        std::cout << "测点名称：" << spectrumData.getMeasurementPointNameString() << std::endl;
        std::cout << "温度矩阵：" << spectrumData.temperatureMatrixWidth 
                  << " x " << spectrumData.temperatureMatrixHeight << std::endl;
        std::cout << "辐射率：" << spectrumData.emissivity << std::endl;
    }
}

/**
 * @brief 示例 4：时间格式解析示例
 */
void example4_TimeFormat()
{
    InfraredSpectrumData spectrumData;
    
    // 设置时间为 2024-04-01 15:30:00.123
    // 格式：YYYYMMDDhhmmssfff = 20240401153000123
    spectrumData.createTime = 20240401153000123LL;
    
    // 解析为可读字符串
    std::string timeStr = spectrumData.getCreateTimeString();
    // 输出："2024-04-01 15:30:00.123"
    
    std::cout << "时间字符串：" << timeStr << std::endl;
}

/**
 * @brief 示例 5：保存照片数据到文件
 */
void example5_SavePhotoData(const InfraredSpectrumData& spectrumData)
{
    // 保存可见光照片
    if (!spectrumData.visibleLightPhotoData.empty())
    {
        FILE* file = fopen("visible_light_photo.jpg", "wb");
        if (file)
        {
            fwrite(spectrumData.visibleLightPhotoData.data(), 
                   1, 
                   spectrumData.visibleLightPhotoData.size(), 
                   file);
            fclose(file);
            std::cout << "可见光照片已保存" << std::endl;
        }
    }
    
    // 保存红外照片
    if (!spectrumData.infraredPhotoData.empty())
    {
        FILE* file = fopen("infrared_photo.jpg", "wb");
        if (file)
        {
            fwrite(spectrumData.infraredPhotoData.data(), 
                   1, 
                   spectrumData.infraredPhotoData.size(), 
                   file);
            fclose(file);
            std::cout << "红外照片已保存" << std::endl;
        }
    }
}

// 主函数示例
int main()
{
    std::cout << "=== 红外图谱数据解析示例 ===" << std::endl;
    
    // 运行示例 1
    std::cout << "\n示例 1：从通信协议解析" << std::endl;
    example1_ParseFromCommunicationProtocol();
    
    // 运行示例 2
    std::cout << "\n示例 2：序列化到字节流" << std::endl;
    example2_SerializeToBytes();
    
    // 运行示例 3
    std::cout << "\n示例 3：直接解析字节流" << std::endl;
    example3_DirectParseFromBytes();
    
    // 运行示例 4
    std::cout << "\n示例 4：时间格式解析" << std::endl;
    example4_TimeFormat();
    
    return 0;
}
