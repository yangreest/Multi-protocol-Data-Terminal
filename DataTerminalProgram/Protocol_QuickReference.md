# CommunicationProtocol 类快速参考

## 📦 类结构

### 公共字段（17 个协议字段）

```cpp
// 固定头部字段（1-11）
uint32_t packetHeader;          // 报文头：0xEB90EB90
uint8_t version;                // 版本号：从 1 开始
uint16_t sequenceNumber;        // 序号：1-65535
uint8_t requestFlag;            // 请求标志：0x00=请求，0x01=响应
uint64_t totalPacketLength;     // 数据包总长度
uint32_t packetTypeCode;        // 报文类型编码
uint8_t compressionFlag;        // 压缩标志：0x00=无压缩，0x01=gzip
uint8_t encryptionFlag;         // 加密标志：0x00=无加密，0x01=加密
uint8_t instrumentVendor;       // 仪器厂商：0x05=FLIR，0x00=其他
std::vector<uint8_t> reserved;  // 备用：15 字节
uint8_t serviceDataFormat;      // 业务数据格式：0x01=XML

// 可变长度字段（12-15）
uint64_t serviceDataLength;     // 业务数据长度
std::vector<uint8_t> serviceData;        // 业务数据（UTF-8）
uint64_t detectionFileLength;   // 检测文件长度
std::vector<uint8_t> detectionFileData;  // 检测文件数据（.dat）

// 校验和尾部（16-17）
uint32_t crc32Checksum;         // CRC32 校验
uint8_t packetTail;             // 报文尾：0x03
```

## 🔧 核心方法

### 1. 从字节流构建

```cpp
// 从原始字节构建协议对象
bool buildFromBytes(const uint8_t* data, size_t length);
bool buildFromBytes(const std::vector<uint8_t>& data);

// 从十六进制字符串构建协议对象
bool buildFromHexString(const std::string& hexString);
bool buildFromHexStringWithSpaces(const std::string& hexString);
```

**示例：**
```cpp
CommunicationProtocol protocol;

// 从字节构建
std::vector<uint8_t> receivedData = tcpClient.receive();
if (protocol.buildFromBytes(receivedData)) {
    // 解析成功
    std::cout << "序号：" << protocol.sequenceNumber;
}

// 从十六进制字符串构建（无空格）
std::string hexString = "eb90eb9001000101000000000000003c...";
if (protocol.buildFromHexString(hexString)) {
    std::cout << "解析成功";
}

// 从带空格的十六进制字符串构建
std::string hexWithSpaces = "eb 90 eb 90 01 00 01...";
if (protocol.buildFromHexStringWithSpaces(hexWithSpaces)) {
    std::cout << "解析成功";
}
```

### 2. 生成字节流（用于 TCP 传输）

```cpp
std::vector<uint8_t> toBytes();

// 转换为十六进制字符串
std::string toHexString() const;
std::string toHexStringWithSeparator(char separator = ' ') const;
```

**示例：**
```cpp
CommunicationProtocol protocol;
protocol.sequenceNumber = 1;
protocol.setServiceData("<xml>data</xml>");

// 生成字节流
std::vector<uint8_t> bytes = protocol.toBytes();
// 自动计算 totalPacketLength 和 crc32Checksum
tcpClient.Write(bytes.data(), bytes.size());

// 生成十六进制字符串
std::string hexString = protocol.toHexString();
std::cout << "十六进制：" << hexString << std::endl;

// 生成带空格的十六进制字符串
std::string hexWithSpaces = protocol.toHexStringWithSeparator(' ');
std::cout << "带空格：" << hexWithSpaces << std::endl;
```

### 3. 设置业务数据

```cpp
void setServiceData(const std::string& data);
void setServiceData(const uint8_t* data, size_t length);
std::string getServiceDataString() const;
```

**示例：**
```cpp
// 设置 XML 数据
protocol.setServiceData("<?xml version=\"1.0\"?><request/>");

// 获取 XML 数据
std::string xml = protocol.getServiceDataString();
```

### 4. 设置检测数据文件

```cpp
void setDetectionFile(const uint8_t* data, size_t length);
void setDetectionFile(const std::vector<uint8_t>& data);
```

**示例：**
```cpp
// 从文件加载
std::vector<uint8_t> fileData = read_dat_file("test.dat");
protocol.setDetectionFile(fileData);
```

### 5. CRC32 验证

```cpp
bool verifyCRC32() const;
```

**示例：**
```cpp
if (protocol.verifyCRC32()) {
    std::cout << "CRC 校验通过";
}
```

## 📝 常用配置值

### 字节序（重要）
**所有多字节字段使用大端模式（Big-Endian / Network Byte Order）**
- uint16: 2 字节，高位字节在前（例如：0x0001 → `00 01`）
- uint32: 4 字节，高位字节在前（例如：0xEB90EB90 → `EB 90 EB 90`）
- uint64: 8 字节，高位字节在前（例如：0x000000000000003C → `00 00 00 00 00 00 00 3C`）

这是网络协议的标准字节序，与 Intel x86 的小端模式不同。

### 请求标志 (requestFlag)
- `0x00` - 请求报文
- `0x01` - 响应报文

### 压缩标志 (compressionFlag)
- `0x00` - 无压缩
- `0x01` - gzip 压缩

### 加密标志 (encryptionFlag)
- `0x00` - 无加密
- `0x01` - 加密

### 仪器厂商 (instrumentVendor)
- `0x05` - FLIR
- `0x00` - 其他厂家

### 业务数据格式 (serviceDataFormat)
- `0x01` - XML 格式

## 🎯 典型使用场景

### 场景 1：发送请求报文

```cpp
CommunicationProtocol request;
request.sequenceNumber = 1;
request.requestFlag = 0x00;  // 请求
request.packetTypeCode = 0x00010001;
request.setServiceData("<?xml version=\"1.0\"?><GetDeviceStatus/>");

std::vector<uint8_t> data = request.toBytes();
tcpClient.Write(data.data(), data.size());
```

### 场景 2：接收并解析响应

```cpp
// 接收数据
std::vector<uint8_t> receivedData = receiveFromTcp();

// 解析协议
CommunicationProtocol response;
if (response.buildFromBytes(receivedData)) {
    // 验证 CRC
    if (!response.verifyCRC32()) {
        std::cerr << "CRC 校验失败！" << std::endl;
        return;
    }
    
    // 检查是否为响应
    if (response.requestFlag == 0x01) {
        std::string xmlData = response.getServiceDataString();
        std::cout << "收到响应：" << xmlData << std::endl;
    }
}
```

### 场景 3：发送带文件的报文

```cpp
CommunicationProtocol upload;
upload.sequenceNumber = 100;
upload.requestFlag = 0x00;
upload.packetTypeCode = 0x00020001;

// 设置 XML 业务数据
upload.setServiceData("<?xml version=\"1.0\"?><UploadFile/>");

// 加载并设置检测数据文件
std::vector<uint8_t> fileData = read_dat_file("measurement.dat");
upload.setDetectionFile(fileData);

// 发送
std::vector<uint8_t> bytes = upload.toBytes();
tcpClient.Write(bytes.data(), bytes.size());
```

### 场景 4：批量发送（序号递增）

``cpp
CommunicationProtocol protocol;
for (int i = 1; i <= 10; i++) {
    protocol.sequenceNumber = i;
    protocol.setServiceData("<data>test</data>");
    
    std::vector<uint8_t> bytes = protocol.toBytes();
    tcpClient.Write(bytes.data(), bytes.size());
    
    std```

## ⚠️ 注意事项

1. **字节序**：所有多字节字段使用小端模式（Little-Endian）
2. **CRC 计算**：自动在 `toBytes()` 中计算，无需手动处理
3. **总长度**：自动计算，包含所有字段（头部 47 字节 + 业务数据 + 检测文件 + CRC 4 字节 + 包尾 1 字节）
4. **备用字段**：固定 15 字节，默认填 0
5. **序号范围**：1-65535，循环使用
6. **业务数据**：必须使用 UTF-8 编码

## 🔍 调试技巧

### 打印所有字段

```cpp
void printProtocolInfo(const CommunicationProtocol& p) {
    std::cout << "=== 协议信息 ===" << std::endl;
    std::cout << "报文头：0x" << std::hex << p.packetHeader << std::endl;
    std::cout << "版本号：" << (int)p.version << std::endl;
    std::cout << "序号：" << p.sequenceNumber << std::endl;
    std::cout << "请求标志：0x" << (int)p.requestFlag << std::endl;
    std::cout << "总长度：" << p.totalPacketLength << std::endl;
    std::cout << "报文类型：0x" << std::hex << p.packetTypeCode << std::endl;
    std::cout << "压缩标志：0x" << (int)p.compressionFlag << std::endl;
    std::cout << "加密标志：0x" << (int)p.encryptionFlag << std::endl;
    std::cout << "厂商代码：0x" << (int)p.instrumentVendor << std::endl;
    std::cout << "数据格式：0x" << (int)p.serviceDataFormat << std::endl;
    std::cout << "业务数据长度：" << p.serviceDataLength << std::endl;
    std::cout << "业务数据：" << p.getServiceDataString() << std::endl;
    std::cout << "检测文件长度：" << p.detectionFileLength << std::endl;
    std::cout << "CRC32: 0x" << std::hex << p.crc32Checksum << std::endl;
    std::cout << "报文尾：0x" << std::hex << (int)p.packetTail << std::endl;
}
```

### 验证字节流

```
// 生成字节流
std::vector<uint8_t> bytes = protocol.toBytes();

// 验证报文头
if (bytes[0] == 0x90 && bytes[1] == 0xEB && 
    bytes[2] == 0x90 && bytes[3] == 0xEB) {
    std::cout << "✓ 报文头正确" << std::endl;
}

// 验证包尾
if (bytes[bytes.size() - 1] == 0x03) {
    std::cout << "✓ 报文尾正确" << std::endl;
}
```

## 📚 相关文件

- `CommunicationProtocol.h` - 协议类定义
- `ProtocolExample.cpp` - 使用示例
- `Protocol_Verification.md` - 字段核对清单
- `CRC32.h` - CRC32 校验算法
- `TcpClient.h` - TCP 客户端通信
