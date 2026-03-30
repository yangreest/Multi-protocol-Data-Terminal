# 十六进制字符串与协议对象转换

## 📋 新增功能

CommunicationProtocol 类现在支持**从十六进制字符串直接构建协议对象**，以及**将协议对象转换为十六进制字符串**。

## 🔧 使用方法

### 1️⃣ 从十六进制字符串构建协议对象

#### 方法一：无空格字符串

```cpp
CommunicationProtocol protocol;

// 您提供的十六进制字符串示例
std::string hexString = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";

if (protocol.buildFromHexString(hexString))
{
    std::cout << "解析成功！" << std::endl;
    std::cout << "报文头：0x" << std::hex << protocol.packetHeader << std::endl;
    std::cout << "序号：" << protocol.sequenceNumber << std::endl;
    std::cout << "请求标志：0x" << (int)protocol.requestFlag << std::endl;
}
```

#### 方法二：带空格的字符串

```cpp
CommunicationProtocol protocol;

// 带空格的十六进制字符串（更易读）
std::string hexWithSpaces = "eb 90 eb 90 01 00 01 01 00 00 00 00 00 00 00 3c "
                            "80 00 00 01 01 00 00 00 00 00 00 00 00 00 00 "
                            "00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 "
                            "00 00 00 00 00 00 00 00 00 00 00 00 00 7d 0d "
                            "3e 06 03";

if (protocol.buildFromHexStringWithSpaces(hexWithSpaces))
{
    std::cout << "解析成功！" << std::endl;
}
```

#### 支持的格式

`buildFromHexStringWithSpaces()` 方法支持多种分隔符：
- 空格：`eb 90 eb 90`
- 横线：`eb-90-eb-90`
- 冒号：`eb:90:eb:90`
- 制表符、换行符等

### 2️⃣ 将协议对象转换为十六进制字符串

#### 方法一：无空格字符串

```cpp
CommunicationProtocol protocol;
protocol.sequenceNumber = 1;
protocol.setServiceData("<?xml version=\"1.0\"?><test/>");

// 生成字节流并转换为十六进制字符串
std::string hexString = protocol.toHexString();
std::cout << "十六进制：" << hexString << std::endl;
// 输出：eb90eb90010001...（连续字符串）
```

#### 方法二：带分隔符的字符串

```cpp
// 生成带空格的十六进制字符串
std::string hexWithSpaces = protocol.toHexStringWithSeparator(' ');
std::cout << "带空格：" << hexWithSpaces << std::endl;
// 输出：eb 90 eb 90 01 00 01...（每个字节用空格分隔）

// 也可以使用其他分隔符
std::string hexWithDash = protocol.toHexStringWithSeparator('-');
std::cout << "带横线：" << hexWithDash << std::endl;
// 输出：eb-90-eb-90-01-00-01...
```

## 📝 完整示例

### 示例 1：解析接收到的十六进制报文

```cpp
#include "CommunicationProtocol.h"
#include <iostream>

int main()
{
    // 接收到的十六进制报文（您提供的示例）
    std::string receivedHex = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";
    
    // 解析报文
    CommunicationProtocol protocol;
    if (protocol.buildFromHexString(receivedHex))
    {
        std::cout << "=== 报文解析成功 ===" << std::endl;
        std::cout << "报文头：0x" << std::hex << protocol.packetHeader << std::endl;
        std::cout << "版本号：" << (int)protocol.version << std::endl;
        std::cout << "序号：" << protocol.sequenceNumber << std::endl;
        std::cout << "请求标志：" << (protocol.requestFlag == 0x00 ? "请求" : "响应") << std::endl;
        std::cout << "总长度：" << protocol.totalPacketLength << std::endl;
        std::cout << "报文类型：0x" << protocol.packetTypeCode << std::endl;
        std::cout << "压缩标志：" << (protocol.compressionFlag == 0x00 ? "无压缩" : "gzip 压缩") << std::endl;
        std::cout << "加密标志：" << (protocol.encryptionFlag == 0x00 ? "无加密" : "加密") << std::endl;
        std::cout << "仪器厂商：0x" << (int)protocol.instrumentVendor << std::endl;
        std::cout << "业务数据格式：0x" << (int)protocol.serviceDataFormat << std::endl;
        std::cout << "业务数据长度：" << protocol.serviceDataLength << std::endl;
        std::cout << "检测文件长度：" << protocol.detectionFileLength << std::endl;
        std::cout << "CRC32: 0x" << protocol.crc32Checksum << std::endl;
        std::cout << "报文尾：0x" << (int)protocol.packetTail << std::endl;
        
        // 验证 CRC
        if (protocol.verifyCRC32())
        {
            std::cout << "✓ CRC32 校验通过" << std::endl;
        }
        else
        {
            std::cout << "⚠ CRC32 校验失败" << std::endl;
        }
    }
    else
    {
        std::cout << "✗ 报文解析失败" << std::endl;
    }
    
    return 0;
}
```

### 示例 2：构造报文并转换为十六进制

```cpp
#include "CommunicationProtocol.h"
#include <iostream>

int main()
{
    // 创建请求报文
    CommunicationProtocol request;
    request.sequenceNumber = 1;
    request.requestFlag = 0x00;  // 请求
    request.packetTypeCode = 0x00010001;
    request.compressionFlag = 0x00;
    request.encryptionFlag = 0x00;
    request.instrumentVendor = 0x05;  // FLIR
    request.setServiceData("<?xml version=\"1.0\"?><GetDeviceStatus/>");
    
    // 转换为十六进制字符串
    std::string hexString = request.toHexString();
    std::cout << "请求报文十六进制：" << std::endl;
    std::cout << hexString << std::endl << std::endl;
    
    // 转换为带空格的十六进制字符串（便于阅读）
    std::string hexWithSpaces = request.toHexStringWithSeparator(' ');
    std::cout << "带空格的十六进制：" << std::endl;
    std::cout << hexWithSpaces << std::endl;
    
    return 0;
}
```

### 示例 3：往返转换验证

```cpp
#include "CommunicationProtocol.h"
#include <iostream>

int main()
{
    // 创建原始协议对象
    CommunicationProtocol original;
    original.sequenceNumber = 100;
    original.requestFlag = 0x01;  // 响应
    original.packetTypeCode = 0x00020001;
    original.setServiceData("<?xml version=\"1.0\"?><Response/>");
    
    // 转换为十六进制字符串
    std::string hexString = original.toHexString();
    std::cout << "原始十六进制：" << hexString << std::endl << std::endl;
    
    // 从十六进制字符串还原
    CommunicationProtocol restored;
    if (restored.buildFromHexString(hexString))
    {
        std::cout << "=== 还原成功 ===" << std::endl;
        std::cout << "序号：" << restored.sequenceNumber << std::endl;
        std::cout << "请求标志：0x" << (int)restored.requestFlag << std::endl;
        std::cout << "报文类型：0x" << restored.packetTypeCode << std::endl;
        std::cout << "业务数据：" << restored.getServiceDataString() << std::endl;
        
        // 验证 CRC
        if (restored.verifyCRC32())
        {
            std::cout << "✓ CRC32 校验通过" << std::endl;
        }
    }
    
    return 0;
}
```

## ⚙️ 实际应用场景

### 场景 1：调试和日志记录

``cpp
// 接收 TCP 数据
std::vector<uint8_t> receivedData = tcpClient.receive();

// 解析协议
CommunicationProtocol protocol;
if (protocol.buildFromBytes(receivedData))
{
    // 记录十六进制日志
    std::string hexLog = protocol.toHexStringWithSeparator(' ');
    std::cout << "收到报文：" << hexLog << std::endl;
    
    // 注意：所有多字节字段使用大端模式（网络字节序）
    // 例如：序号 0x0001 在字节流中为 00 01
    //       报文头 0xEB90EB90 在字节流中为 EB 90 EB 90
    
    // 处理业务逻辑...
}
```

### 场景 2：从配置文件加载报文模板

``cpp
// 配置文件中的报文模板（十六进制字符串）
std::string configHex = "eb90eb9001000101000000000000003c...";

// 加载报文模板
CommunicationProtocol template;
if (template.buildFromHexString(configHex))
{
    // 使用模板报文...
}
```

### 场景 3：网络抓包分析

``cpp
// 从 Wireshark 等工具复制的十六进制数据
std::string capturedHex = "eb 90 eb 90 01 00 01 01 00 00 00 00 00 00 00 3c "
                          "80 00 00 01 01 00 00 00 00 00 00 00 00 00 00 "
                          "00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 "
                          "00 00 00 00 00 00 00 00 00 00 00 00 00";

// 解析抓包数据
CommunicationProtocol protocol;
if (protocol.buildFromHexStringWithSpaces(capturedHex))
{
    // 分析报文内容...
}
```

## 🔍 注意事项

1. **字符串格式**：
   - `buildFromHexString()` - 不接受空格或其他分隔符
   - `buildFromHexStringWithSpaces()` - 自动忽略空格、横线、冒号等分隔符

2. **大小写不敏感**：
   - `buildFromHexString()` 和 `buildFromHexStringWithSpaces()` 都支持大小写
   - `"EB90EB90"` 和 `"eb90eb90"` 效果相同

3. **字符串长度**：
   - 必须是偶数（每个字节由两个十六进制字符表示）
   - 如果长度为奇数，`buildFromHexString()` 将返回 false

4. **CRC 校验**：
   - 从字符串构建时，会保留原始的 CRC32 值
   - 可以使用 `verifyCRC32()` 验证数据完整性

5. **编码格式**：
   - 只支持标准的十六进制字符：0-9, A-F, a-f
   - 不支持 0x 前缀

## 📊 性能提示

- `toHexString()` 比 `toHexStringWithSeparator()` 稍快（不需要添加分隔符）
- `buildFromHexStringWithSpaces()` 会先清理字符串，比 `buildFromHexString()` 多一次遍历
- 对于大量数据处理，建议使用无空格版本以提高性能

## 🎯 最佳实践

1. **调试输出**：使用 `toHexStringWithSeparator(' ')` 便于人工阅读
2. **网络传输**：使用 `toBytes()` 直接发送字节流
3. **日志记录**：使用 `toHexString()` 节省存储空间
4. **配置文件**：使用带空格的十六进制字符串便于维护
5. **测试用例**：使用十六进制字符串可以精确控制每个字段的值
