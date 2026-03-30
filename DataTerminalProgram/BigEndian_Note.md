# 大端模式（Big-Endian）说明

## 📋 字节序定义

CommunicationProtocol 类中**所有多字节字段**使用**大端模式**（Big-Endian），也称为**网络字节序**。

### 大端模式 vs 小端模式

- **大端模式（Big-Endian）**：高位字节存储在低地址（左边）
- **小端模式（Little-Endian）**：低位字节存储在低地址（左边）

**网络协议标准**：使用大端模式（RFC 1700）

## 🔍 字段字节序示例

### 1. uint16 类型（2 字节）

```cpp
uint16_t value = 0x0102;
```

**大端模式字节序：**
```
地址：  0x00  0x01
数据：  0x01  0x02
        ↑
      高位在前
```

**小端模式字节序（对比）：**
```
地址：  0x00  0x01
数据：  0x02  0x01
        ↑
      低位在前
```

### 2. uint32 类型（4 字节）

```cpp
uint32_t value = 0xEB90EB90;
```

**大端模式字节序：**
```
地址：  0x00  0x01  0x02  0x03
数据：  0xEB  0x90  0xEB  0x90
        ↑
      高位在前
```

**十六进制字符串表示：** `EB 90 EB 90`

### 3. uint64 类型（8 字节）

```cpp
uint64_t value = 0x0102030405060708;
```

**大端模式字节序：**
```
地址：  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
数据：  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08
        ↑
      高位在前
```

**十六进制字符串表示：** `01 02 03 04 05 06 07 08`

## 📊 协议字段字节序详解

### 完整报文示例

假设创建以下协议对象：

```cpp
CommunicationProtocol protocol;
protocol.packetHeader = 0xEB90EB90;        // 报文头
protocol.version = 1;                       // 版本号
protocol.sequenceNumber = 0x0001;           // 序号
protocol.requestFlag = 0x00;                // 请求标志
protocol.totalPacketLength = 0x000000000000003C; // 总长度 (60)
protocol.packetTypeCode = 0x00010001;       // 报文类型
protocol.compressionFlag = 0x00;            // 无压缩
protocol.encryptionFlag = 0x00;             // 无加密
protocol.instrumentVendor = 0x05;           // FLIR
// ... 其他字段
```

### 字节流布局（大端模式）

```
偏移    字段              值 (十六进制)           字节序
----    ----              ----------------       --------
0-3     报文头            0xEB90EB90            EB 90 EB 90
4       版本号            0x01                  01
5-6     序号              0x0001                00 01
7       请求标志          0x00                  00
8-15    总长度            0x000000000000003C    00 00 00 00 00 00 00 3C
16-19   报文类型          0x00010001            00 01 00 01
20      压缩标志          0x00                  00
21      加密标志          0x00                  00
22      仪器厂商          0x05                  05
23-37   备用              0x00 (×15)            00 00 ... 00
38      业务数据格式      0x01                  01
39-46   业务数据长度      0x0000000000000000    00 00 00 00 00 00 00 00
47-...  业务数据          (可变长度)
...     检测文件长度      0x0000000000000000    00 00 00 00 00 00 00 00
...     检测数据文件      (可变长度)
...     CRC32             (4 字节)
...     报文尾            0x03                  03
```

### 完整字节流示例

```
EB 90 EB 90  01  00 01  00  00 00 00 00 00 00 3C  00 01 00 01  
00 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01  
00 00 00 00 00 00 00 00  
(业务数据)  
00 00 00 00 00 00 00 00  
(检测数据)  
(CRC32)  
03
```

## 🔧 实现细节

### 读取函数（大端模式）

```cpp
// uint16 读取
static uint16_t readUint16(const uint8_t* data, size_t offset)
{
    return (static_cast<uint16_t>(data[offset]) << 8) | 
           static_cast<uint16_t>(data[offset + 1]);
}
// 示例：data[5]=0x00, data[6]=0x01 → 返回 0x0001
```

```cpp
// uint32 读取
static uint32_t readUint32(const uint8_t* data, size_t offset)
{
    return (static_cast<uint32_t>(data[offset]) << 24) | 
           (static_cast<uint32_t>(data[offset + 1]) << 16) | 
           (static_cast<uint32_t>(data[offset + 2]) << 8) | 
           static_cast<uint32_t>(data[offset + 3]);
}
// 示例：data[0]=0xEB, data[1]=0x90, data[2]=0xEB, data[3]=0x90 → 返回 0xEB90EB90
```

```cpp
// uint64 读取
static uint64_t readUint64(const uint8_t* data, size_t offset)
{
    return (static_cast<uint64_t>(data[offset]) << 56) | 
           (static_cast<uint64_t>(data[offset + 1]) << 48) | 
           (static_cast<uint64_t>(data[offset + 2]) << 40) | 
           (static_cast<uint64_t>(data[offset + 3]) << 32) | 
           (static_cast<uint64_t>(data[offset + 4]) << 24) | 
           (static_cast<uint64_t>(data[offset + 5]) << 16) | 
           (static_cast<uint64_t>(data[offset + 6]) << 8) | 
           static_cast<uint64_t>(data[offset + 7]);
}
// 示例：data[8]=0x00, data[9]=0x00, ..., data[15]=0x3C → 返回 0x000000000000003C
```

### 写入函数（大端模式）

```cpp
// uint16 写入
static void writeUint16(std::vector<uint8_t>& buffer, uint16_t value)
{
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));      // 高字节
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));             // 低字节
}
// 示例：value=0x0102 → buffer: 01 02
```

```cpp
// uint32 写入
static void writeUint32(std::vector<uint8_t>& buffer, uint32_t value)
{
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));     // 最高字节
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));             // 最低字节
}
// 示例：value=0xEB90EB90 → buffer: EB 90 EB 90
```

```cpp
// uint64 写入
static void writeUint64(std::vector<uint8_t>& buffer, uint64_t value)
{
    buffer.push_back(static_cast<uint8_t>((value >> 56) & 0xFF));     // 最高字节
    buffer.push_back(static_cast<uint8_t>((value >> 48) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 40) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 32) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));             // 最低字节
}
// 示例：value=0x0102030405060708 → buffer: 01 02 03 04 05 06 07 08
```

## ✅ 验证方法

### 方法 1：使用 toHexString() 查看

```cpp
CommunicationProtocol protocol;
protocol.sequenceNumber = 0x0102;
protocol.packetTypeCode = 0x01020304;

std::string hexString = protocol.toHexString();
std::cout << hexString << std::endl;

// 大端模式输出应包含：... 01 02 ... 01 02 03 04 ...
```

### 方法 2：直接检查字节流

```cpp
std::vector<uint8_t> bytes = protocol.toBytes();

// 检查报文头 (偏移 0-3)
std::cout << "报文头字节序：";
for (size_t i = 0; i < 4; i++) {
    printf("%02X ", bytes[i]);
}
// 应输出：EB 90 EB 90

// 检查序号 (偏移 5-6)
std::cout << "序号字节序：";
for (size_t i = 5; i < 7; i++) {
    printf("%02X ", bytes[i]);
}
// 应输出：00 01 (如果 sequenceNumber = 0x0001)
```

### 方法 3：往返验证

```cpp
// 创建协议对象
CommunicationProtocol original;
original.sequenceNumber = 0x1234;
original.packetTypeCode = 0x12345678;

// 转换为字节流
std::vector<uint8_t> bytes = original.toBytes();

// 从字节流还原
CommunicationProtocol restored;
restored.buildFromBytes(bytes);

// 验证值是否相同
if (original.sequenceNumber == restored.sequenceNumber &&
    original.packetTypeCode == restored.packetTypeCode) {
    std::cout << "✓ 大端模式字节序正确" << std::endl;
}
```

## ⚠️ 注意事项

### 1. 与 x86 架构的区别

- **x86/x64 CPU**（Intel、AMD）使用**小端模式**
- **网络协议**使用**大端模式**
- 在本地存储和处理时需要注意转换

### 2. 跨平台兼容性

使用大端模式（网络字节序）确保：
- ✅ 不同架构的 CPU 之间可以正确通信
- ✅ 符合网络协议标准
- ✅ 与现有的网络设备兼容

### 3. 调试技巧

当解析十六进制字符串时：
```cpp
std::string hexString = "EB90EB900001...";

CommunicationProtocol protocol;
protocol.buildFromHexString(hexString);

// 如果报文头应该是 0xEB90EB90
// 但解析结果不正确，可能是字节序问题
std::cout << "报文头：0x" << std::hex << protocol.packetHeader << std::endl;
// 正确输出应为：0xEB90EB90
```

### 4. 常见错误

❌ **错误：使用小端模式解析**
```cpp
// 错误示例（小端模式）
uint32_t value = *(uint32_t*)(data + offset);  // x86 上这是小端
```

✅ **正确：使用大端模式解析**
```cpp
// 正确示例（大端模式）
uint32_t value = (static_cast<uint32_t>(data[offset]) << 24) |
                 (static_cast<uint32_t>(data[offset + 1]) << 16) |
                 (static_cast<uint32_t>(data[offset + 2]) << 8) |
                 static_cast<uint32_t>(data[offset + 3]);
```

## 📚 参考资料

- **RFC 1700** - Assigned Numbers (字节序标准)
- **Q/GDW XXXXX.3-2022** - 通信报文通用格式
- **TCP/IP 协议族** - 使用大端模式作为标准字节序

## 🎯 总结

| 特性 | 说明 |
|------|------|
| **字节序** | 大端模式（Big-Endian） |
| **别名** | 网络字节序（Network Byte Order） |
| **应用范围** | 所有多字节字段（uint16、uint32、uint64） |
| **优点** | 符合网络标准、跨平台兼容、易于调试 |
| **验证方法** | toHexString()、直接检查字节流、往返测试 |

**所有字段严格按照大端模式实现，确保与网络协议标准完全一致！** ✅
