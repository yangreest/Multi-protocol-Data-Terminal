/**
 * @brief 使用示例：解析图像数据到 ImageData
 * 
 * 示例代码展示了如何使用 ImageData 类解析图像数据
 */

#include "ImageData.h"
#include <iostream>
#include <fstream>

/**
 * @brief 示例 1：从字节流解析图像数据
 */
void example1_ParseFromBytes()
{
    // 模拟从文件或网络接收的图像数据
    // 这里创建一个符合格式的测试数据
    std::vector<uint8_t> testData;
    
    // 1. 文件长度 (4 字节) - 假设总长度为 1024 字节（包含 CRC）
    testData.push_back(0x00);
    testData.push_back(0x00);
    testData.push_back(0x04);
    testData.push_back(0x00); // 1024
    
    // 2. 规范版本号 (4 字节) - 版本 1.0.0.0
    testData.push_back(0x01);
    testData.push_back(0x00);
    testData.push_back(0x00);
    testData.push_back(0x00);
    
    // 3. 文件生成时间 (8 字节) - 2024-04-01 15:30:00.123
    // 格式：YYYYMMDDhhmmssfff = 20240401153000123
    testData.push_back(0x00);
    testData.push_back(0x47);
    testData.push_back(0x8A);
    testData.push_back(0x5E);
    testData.push_back(0x7C);
    testData.push_back(0x61);
    testData.push_back(0x40);
    testData.push_back(0x7B);
    
    // 4. 站点名称 (118 字节) - "Test Station"
    std::string stationName = "Test Station";
    for (size_t i = 0; i < stationName.length(); i++)
    {
        testData.push_back(0x00); // 高位字节
        testData.push_back(stationName[i]); // 低位字节
    }
    // 添加结束符
    testData.push_back(0x00);
    testData.push_back(0x00);
    // 填充剩余字节
    while (testData.size() < 134)
    {
        testData.push_back(0x00);
    }
    
    // 创建 ImageData 对象并解析
    ImageData imageData;
    
    if (imageData.parseFromBytes(testData))
    {
        std::cout << "=== 解析成功 ===" << std::endl;
        
        // 1. 基本信息
        std::cout << "文件长度：" << imageData.fileLength << " 字节" << std::endl;
        std::cout << "规范版本号：" << imageData.getVersionString() << std::endl;
        std::cout << "生成时间：" << imageData.getCreateTimeString() << std::endl;
        std::cout << "站点名称：" << imageData.getStationNameString() << std::endl;
    }
    else
    {
        std::cerr << "解析图像数据失败！" << std::endl;
    }
}

/**
 * @brief 示例 2：从文件读取并解析图像数据
 */
void example2_ParseFromFile()
{
    // 假设有一个图像数据文件
    const char* filename = "image_data.bin";
    
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cout << "无法打开文件：" << filename << std::endl;
        std::cout << "（这是正常的，因为这是一个示例）" << std::endl;
        return;
    }
    
    // 获取文件大小
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 读取文件内容
    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        // 解析图像数据
        ImageData imageData;
        
        if (imageData.parseFromBytes(buffer))
        {
            std::cout << "=== 从文件解析成功 ===" << std::endl;
            std::cout << "文件：" << filename << std::endl;
            std::cout << "文件长度：" << imageData.fileLength << std::endl;
            std::cout << "版本号：" << imageData.getVersionString() << std::endl;
            std::cout << "生成时间：" << imageData.getCreateTimeString() << std::endl;
            std::cout << "站点名称：" << imageData.getStationNameString() << std::endl;
        }
        else
        {
            std::cerr << "解析图像数据失败！" << std::endl;
        }
    }
    
    file.close();
}

/**
 * @brief 示例 3：创建并序列化图像数据
 */
void example3_CreateAndSerialize()
{
    // 创建 ImageData 对象
    ImageData imageData;
    
    // 设置各个字段
    imageData.fileLength = 1024; // 包含 CRC 的总长度
    
    // 设置版本号（两种方式）
    imageData.versionNumber = {0x01, 0x00, 0x00, 0x00}; // 方式 1：直接设置字节数组
    // imageData.setVersionFromString("1.0.0.0"); // 方式 2：从字符串设置
    
    // 设置生成时间（两种方式）
    imageData.createTime = 20240401153000123LL; // 方式 1：直接设置 int64
    // imageData.setCreateTimeFromString("2024-04-01 15:30:00.123"); // 方式 2：从字符串设置
    
    // 设置站点名称
    imageData.setStationName("1000kV 泉城变电站");
    
    // 序列化为字节流
    std::vector<uint8_t> bytes = imageData.toBytes();
    
    std::cout << "=== 序列化成功 ===" << std::endl;
    std::cout << "序列化后数据大小：" << bytes.size() << " 字节" << std::endl;
    std::cout << "数据内容（前 20 字节）:" << std::endl;
    
    for (size_t i = 0; i < std::min(bytes.size(), (size_t)20); i++)
    {
        printf("%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // 可以将 bytes 保存到文件或通过网络发送
}

/**
 * @brief 示例 4：使用原始指针解析数据
 */
void example4_ParseFromRawPointer()
{
    // 模拟从某个数据缓冲区解析
    uint8_t rawData[] = {
        // 文件长度：1024
        0x00, 0x00, 0x04, 0x00,
        // 版本号：1.0.0.0
        0x01, 0x00, 0x00, 0x00,
        // 时间：2024-04-01 15:30:00.123
        0x00, 0x47, 0x8A, 0x5E, 0x7C, 0x61, 0x40, 0x7B,
        // 站点名称：Test（示例）
        0x00, 0x54, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74
    };
    
    // 填充剩余字节到 134 字节
    std::vector<uint8_t> buffer(rawData, rawData + sizeof(rawData));
    while (buffer.size() < 134)
    {
        buffer.push_back(0x00);
    }
    
    ImageData imageData;
    
    // 使用原始指针方式解析
    if (imageData.parseFromBytes(buffer.data(), buffer.size()))
    {
        std::cout << "=== 使用原始指针解析成功 ===" << std::endl;
        std::cout << "文件长度：" << imageData.fileLength << std::endl;
        std::cout << "版本号：" << imageData.getVersionString() << std::endl;
        std::cout << "生成时间：" << imageData.getCreateTimeString() << std::endl;
        std::cout << "站点名称：" << imageData.getStationNameString() << std::endl;
    }
}

/**
 * @brief 示例 5：从通信协议中解析图像数据
 */
void example5_ParseFromCommunicationProtocol()
{
    // 假设从通信协议接收到数据
    // 例如：从串口、网络等接收到的完整数据帧
    
    // 这里模拟接收到的数据
    std::vector<uint8_t> receivedData;
    
    // 填充测试数据（参考示例 1）
    receivedData.push_back(0x00);
    receivedData.push_back(0x00);
    receivedData.push_back(0x04);
    receivedData.push_back(0x00); // 文件长度
    receivedData.push_back(0x01);
    receivedData.push_back(0x00);
    receivedData.push_back(0x00);
    receivedData.push_back(0x00); // 版本号
    receivedData.push_back(0x00);
    receivedData.push_back(0x47);
    receivedData.push_back(0x8A);
    receivedData.push_back(0x5E);
    receivedData.push_back(0x7C);
    receivedData.push_back(0x61);
    receivedData.push_back(0x40);
    receivedData.push_back(0x7B); // 时间
    std::string stationName = "泉城变电站";
    for (size_t i = 0; i < stationName.length(); i++)
    {
        receivedData.push_back(0x00);
        receivedData.push_back(stationName[i]);
    }
    receivedData.push_back(0x00);
    receivedData.push_back(0x00);
    while (receivedData.size() < 134)
    {
        receivedData.push_back(0x00);
    }
    
    // 解析数据
    ImageData imageData;
    
    if (imageData.parseFromBytes(receivedData))
    {
        std::cout << "=== 从通信协议解析成功 ===" << std::endl;
        std::cout << "文件长度：" << imageData.fileLength << std::endl;
        std::cout << "规范版本：" << imageData.getVersionString() << std::endl;
        std::cout << "生成时间：" << imageData.getCreateTimeString() << std::endl;
        std::cout << "站点名称：" << imageData.getStationNameString() << std::endl;
        
        // 可以进一步处理或显示图像数据
    }
    else
    {
        std::cerr << "解析失败！" << std::endl;
    }
}

/**
 * @brief 主函数 - 运行所有示例
 */
int main()
{
    std::cout << "========== 示例 1：从字节流解析 ==========" << std::endl;
    example1_ParseFromBytes();
    std::cout << std::endl;
    
    std::cout << "========== 示例 2：从文件解析 ==========" << std::endl;
    example2_ParseFromFile();
    std::cout << std::endl;
    
    std::cout << "========== 示例 3：创建并序列化 ==========" << std::endl;
    example3_CreateAndSerialize();
    std::cout << std::endl;
    
    std::cout << "========== 示例 4：使用原始指针解析 ==========" << std::endl;
    example4_ParseFromRawPointer();
    std::cout << std::endl;
    
    std::cout << "========== 示例 5：从通信协议解析 ==========" << std::endl;
    example5_ParseFromCommunicationProtocol();
    std::cout << std::endl;
    
    return 0;
}
