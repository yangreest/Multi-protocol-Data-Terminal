/**
 * @brief 简单的测试程序
 * 演示如何从数据流中构建 ImageData 对象
 */

#include "ImageData.h"
#include <iostream>
#include <vector>

int main()
{
    std::cout << "=== 图像数据解析类测试 ===" << std::endl << std::endl;
    
    // 创建一个测试数据流（模拟从设备或文件接收的数据）
    std::vector<uint8_t> dataStream;
    
    // 1. 文件长度：假设总长度为 1024 字节（包含 CRC）
    // 大端模式：0x00000400 = 1024
    dataStream.push_back(0x00);
    dataStream.push_back(0x00);
    dataStream.push_back(0x04);
    dataStream.push_back(0x00);
    
    // 2. 规范版本号：1.0.0.0
    dataStream.push_back(0x01);
    dataStream.push_back(0x00);
    dataStream.push_back(0x00);
    dataStream.push_back(0x00);
    
    // 3. 文件生成时间：2024-04-01 15:30:00.123
    // 格式：YYYYMMDDhhmmssfff = 20240401153000123
    // 大端模式
    dataStream.push_back(0x00);
    dataStream.push_back(0x47);
    dataStream.push_back(0x8A);
    dataStream.push_back(0x5E);
    dataStream.push_back(0x7C);
    dataStream.push_back(0x61);
    dataStream.push_back(0x40);
    dataStream.push_back(0x7B);
    
    // 4. 站点名称："1000kV 泉城变电站"（UNICODE 编码）
    // 这里简化为英文 "Test Station"
    std::string stationName = "Test Station";
    for (char ch : stationName)
    {
        dataStream.push_back(0x00);  // 高位字节
        dataStream.push_back(ch);    // 低位字节
    }
    // 添加结束符 0x0000
    dataStream.push_back(0x00);
    dataStream.push_back(0x00);
    
    // 填充到 118 字节
    while (dataStream.size() < 134)
    {
        dataStream.push_back(0x00);
    }
    
    std::cout << "测试数据流大小：" << dataStream.size() << " 字节" << std::endl;
    std::cout << "数据流内容（前 20 字节）:" << std::endl;
    for (size_t i = 0; i < 20 && i < dataStream.size(); i++)
    {
        printf("%02X ", dataStream[i]);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
    
    // 从数据流构建 ImageData 对象
    ImageData imageData;
    
    if (imageData.parseFromBytes(dataStream))
    {
        std::cout << "✓ 解析成功！" << std::endl << std::endl;
        
        std::cout << "=== 解析结果 ===" << std::endl;
        std::cout << "文件长度：" << imageData.fileLength << " 字节" << std::endl;
        std::cout << "规范版本号：" << imageData.getVersionString() << std::endl;
        std::cout << "文件生成时间：" << imageData.getCreateTimeString() << std::endl;
        std::cout << "站点名称：" << imageData.getStationNameString() << std::endl;
        std::cout << std::endl;
        
        // 序列化回字节流
        std::vector<uint8_t> serializedData = imageData.toBytes();
        
        std::cout << "=== 序列化结果 ===" << std::endl;
        std::cout << "序列化后大小：" << serializedData.size() << " 字节" << std::endl;
        std::cout << "序列化数据（前 20 字节）:" << std::endl;
        for (size_t i = 0; i < 20 && i < serializedData.size(); i++)
        {
            printf("%02X ", serializedData[i]);
            if ((i + 1) % 16 == 0) std::cout << std::endl;
        }
        std::cout << std::endl << std::endl;
        
        // 验证序列化和反序列化的一致性
        if (serializedData == dataStream)
        {
            std::cout << "✓ 序列化数据与原始数据一致！" << std::endl;
        }
        else
        {
            std::cout << " 序列化数据与原始数据不完全一致（可能是填充字节的差异）" << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "=== 测试完成 ===" << std::endl;
        
        return 0;
    }
    else
    {
        std::cerr << "✗ 解析失败！" << std::endl;
        return 1;
    }
}
