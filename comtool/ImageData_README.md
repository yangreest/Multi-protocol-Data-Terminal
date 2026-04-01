# 图像数据格式解析类 (ImageData)

## 概述

`ImageData` 类用于解析和序列化符合文件数据格式规范（表 2）的图像数据。该类支持从字节流中解析图像数据，也可以将图像数据序列化为字节流以便存储或传输。

## 数据格式说明

### 文件头部结构（512 字节）

根据文件数据格式规范（表 2），图像数据文件头部包含以下字段：

| 序号 | 数据项 | 数据类型 | 长度 | 字节顺序 | 备注 | 必备/可选 |
|------|--------|----------|------|----------|------|-----------|
| 1 | 文件长度 L | int32 | 4 字节 | [0:3] | 文件长度，含 CRC 校验 | 必备 |
| 2 | 规范版本号 | uint8 | 4 字节 | [4:7] | 形如 X.X.X.X | 必备 |
| 3 | 文件生成时间 | int64 | 8 字节 | [8:15] | 格式：YYYYMMDDhhmmssfff | 必备 |
| 4 | 站点名称 | char | 118 字节 | [16:133] | UNICODE 编码，0x0000 结尾 | 可选 |
| 5 | 站点编码 | char | 42 字节 | [134:175] | ASCII 编码，\0 结尾 | 可选 |
| 6 | 天气 | uint8 | 1 字节 | [176:176] | 0xFF:未记录，0x01:晴等 | 可选 |
| 7 | 温度 | float | 4 字节 | [177:180] | 环境温度，单位°C | 可选 |
| 8 | 湿度 | int8 | 1 字节 | [181:181] | 环境湿度，单位% | 可选 |
| 9 | 仪器厂家 | char | 32 字节 | [182:213] | UNICODE 编码，0x0000 结尾 | 必备 |
| 10 | 仪器型号 | char | 32 字节 | [214:245] | UNICODE 编码，0x0000 结尾 | 必备 |
| 11 | 仪器版本号 | uint8 | 4 字节 | [246:249] | 形如 X.X.X.X | 可选 |
| 12 | 仪器序列号 | char | 32 字节 | [250:281] | ASCII 编码，\0 结尾 | 必备 |
| 13 | 系统频率 | float | 4 字节 | [282:285] | 单位 Hz | 必备 |
| 14 | 图谱数量 N | int16 | 2 字节 | [286:287] | 文件中包含的图谱数量 | 必备 |
| 15 | 经度 | double | 8 字节 | [288:295] | 0 代表不支持 | 可选 |
| 16 | 纬度 | double | 8 字节 | [296:303] | 0 代表不支持 | 可选 |
| 17 | 海拔 | int32 | 4 字节 | [304:307] | 单位 m，0 代表不支持 | 可选 |
| 18 | 预留 | 自定义 | 204 字节 | [308:511] | 厂家自定义可选字段 | 可选 |

### 图谱数据区域

- 位置：[512 : L-36]
- 内容：依次存放同一检测方法的多类型数据
- 长度：fileLength - 512 - 36

### 文件尾部结构（36 字节）

| 序号 | 数据项 | 数据类型 | 长度 | 字节顺序 | 备注 | 必备/可选 |
|------|--------|----------|------|----------|------|-----------|
| 19 | 预留 | - | 32 字节 | [L-36:L-5] | 预留字段 | 必备 |
| 20 | CRC32 | int32 | 4 字节 | [L-4:L-1] | 数据校验，使用 CRC32 算法 | 必备 |

**注意：**
- 所有多字节整数类型均采用**大端模式**（Big-Endian）
- 字符串采用**UNICODE**编码（UTF-16 BE）或**ASCII**编码
- 字节顺序严格按照定义顺序排列

## 使用方法

### 1. 从字节流解析

```cpp
#include "ImageData.h"
#include <vector>

// 假设有一个包含图像数据的字节流
std::vector<uint8_t> data = ...; // 从文件、网络或串口接收的数据

// 创建 ImageData 对象
ImageData imageData;

// 解析字节流
if (imageData.parseFromBytes(data))
{
    // 解析成功，访问各个字段
    printf("文件长度：%d\n", imageData.fileLength);
    printf("版本号：%s\n", imageData.getVersionString().c_str());
    printf("生成时间：%s\n", imageData.getCreateTimeString().c_str());
    printf("站点名称：%s\n", imageData.getStationNameString().c_str());
    printf("站点编码：%s\n", imageData.getStationCodeString().c_str());
    printf("天气：%s\n", imageData.getWeatherDescription().c_str());
    printf("温度：%.1f°C\n", imageData.temperature);
    printf("湿度：%d%%\n", imageData.humidity);
    printf("仪器厂家：%s\n", imageData.getInstrumentManufacturerString().c_str());
    printf("仪器型号：%s\n", imageData.getInstrumentModelString().c_str());
    printf("仪器序列号：%s\n", imageData.getInstrumentSerialNumberString().c_str());
    printf("系统频率：%.1fHz\n", imageData.systemFrequency);
    printf("图谱数量：%d\n", imageData.spectrumCount);
    printf("位置信息：%s\n", imageData.getLocationString().c_str());
}
else
{
    printf("解析失败！\n");
}
```

### 2. 创建并序列化数据

```cpp
ImageData imageData;

// 设置基本信息
imageData.fileLength = 1024;
imageData.setVersionFromString("1.0.0.0");
imageData.createTime = 20240401153000123LL;
imageData.setStationName("1000kV 泉城变电站");
imageData.setStationCode("A1230000000000000");

// 设置环境信息
imageData.weather = 0x01; // 晴
imageData.temperature = 25.5f;
imageData.humidity = 60;

// 设置仪器信息
imageData.setInstrumentManufacturer("测试仪器公司");
imageData.setInstrumentModel("TEST-2000");
imageData.setInstrumentVersionFromString("2.0.0.0");
imageData.setInstrumentSerialNumber("S1230000000000000");
imageData.systemFrequency = 50.0f;
imageData.spectrumCount = 1;

// 设置地理位置
imageData.longitude = 116.407526;
imageData.latitude = 39.904030;
imageData.altitude = 50;

// 设置图谱数据
std::vector<uint8_t> spectrumBytes = ...; // 图谱数据
imageData.spectrumData.push_back(spectrumBytes);

// 计算并设置 CRC32
// imageData.crc32 = calculateCRC32(...);

// 序列化为字节流
std::vector<uint8_t> bytes = imageData.toBytes();

// 保存到文件
std::ofstream outFile("image_data.bin", std::ios::binary);
outFile.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
outFile.close();
```

### 3. 设置和获取辅助信息

```cpp
ImageData imageData;

// 使用辅助方法设置字段
imageData.setStationName("泉城变电站");
imageData.setStationCode("A1230000000000000");
imageData.setInstrumentManufacturer("测试仪器公司");
imageData.setInstrumentModel("TEST-2000");
imageData.setInstrumentSerialNumber("S1230000000000000");

// 使用辅助方法获取格式化信息
std::string version = imageData.getVersionString();
std::string timeStr = imageData.getCreateTimeString();
std::string weatherDesc = imageData.getWeatherDescription();
std::string location = imageData.getLocationString();
```

## API 参考

### 公共字段

#### 基本信息
```cpp
int32_t fileLength;              // 文件长度（含 CRC 校验）
std::vector<uint8_t> versionNumber; // 规范版本号（4 字节）
int64_t createTime;              // 文件生成时间
```

#### 站点信息
```cpp
std::vector<uint8_t> stationName;   // 站点名称（118 字节，UNICODE）
std::vector<uint8_t> stationCode;   // 站点编码（42 字节，ASCII）
```

#### 环境信息
```cpp
uint8_t weather;                 // 天气代码
float temperature;               // 环境温度（°C）
int8_t humidity;                 // 环境湿度（%）
```

#### 仪器信息
```cpp
std::vector<uint8_t> instrumentManufacturer; // 仪器厂家（32 字节，UNICODE）
std::vector<uint8_t> instrumentModel;        // 仪器型号（32 字节，UNICODE）
std::vector<uint8_t> instrumentVersion;      // 仪器版本号（4 字节）
std::vector<uint8_t> instrumentSerialNumber; // 仪器序列号（32 字节，ASCII）
```

#### 系统参数
```cpp
float systemFrequency;           // 系统频率（Hz）
int16_t spectrumCount;           // 图谱数量
```

#### 地理位置
```cpp
double longitude;                // 经度
double latitude;                 // 纬度
int32_t altitude;                // 海拔（m）
```

#### 其他字段
```cpp
std::vector<uint8_t> reserved;   // 预留字段（204 字节）
std::vector<std::vector<uint8_t>> spectrumData; // 图谱数据
std::vector<uint8_t> tailReserved; // 文件尾部预留（32 字节）
int32_t crc32;                   // CRC32 校验
```

### 解析方法

```cpp
// 从字节流解析
bool parseFromBytes(const std::vector<uint8_t>& data);

// 从原始指针解析
bool parseFromBytes(const uint8_t* data, size_t size);
```

### 序列化方法

```cpp
// 序列化为字节流
std::vector<uint8_t> toBytes() const;
```

### 辅助方法

#### 获取格式化信息
```cpp
std::string getVersionString() const;                    // 版本号 X.X.X.X
std::string getCreateTimeString() const;                 // 时间 YYYY-MM-DD HH:mm:ss.fff
std::string getStationNameString() const;                // 站点名称
std::string getStationCodeString() const;                // 站点编码
std::string getWeatherDescription() const;               // 天气描述
std::string getInstrumentManufacturerString() const;     // 仪器厂家
std::string getInstrumentModelString() const;            // 仪器型号
std::string getInstrumentVersionString() const;          // 仪器版本号 X.X.X.X
std::string getInstrumentSerialNumberString() const;     // 仪器序列号
std::string getLocationString() const;                   // 位置信息
```

#### 设置字段
```cpp
void setStationName(const std::string& name);                           // 设置站点名称
void setStationCode(const std::string& code);                           // 设置站点编码
void setInstrumentManufacturer(const std::string& name);               // 设置仪器厂家
void setInstrumentModel(const std::string& model);                     // 设置仪器型号
void setInstrumentSerialNumber(const std::string& serial);             // 设置仪器序列号
bool setVersionFromString(const std::string& version);                 // 设置版本号
bool setInstrumentVersionFromString(const std::string& version);       // 设置仪器版本号
bool setCreateTimeFromString(const std::string& timeStr);              // 设置生成时间
```

## 天气代码说明

| 代码 | 含义 |
|------|------|
| 0xFF | 未记录 |
| 0x01 | 晴 |
| 0x02 | 阴 |
| 0x03 | 雨 |
| 0x04 | 雪 |
| 0x05 | 雾 |
| 0x06 | 雷雨 |
| 0x07 | 多云 |

## 注意事项

1. **字节序**：所有多字节整数均采用大端模式（Big-Endian），解析时需要注意字节序转换。

2. **字符串编码**：
   - 站点名称、仪器厂家、仪器型号使用 UNICODE 编码（UTF-16 BE）
   - 站点编码、仪器序列号使用 ASCII 编码
   - 类中提供了自动转换方法

3. **最小长度**：图像数据的最小长度为 548 字节（512 字节头部 + 36 字节尾部），如果数据长度小于此值，解析将失败。

4. **时间格式**：生成时间为 int64 类型，格式为 YYYYMMDDhhmmssfff。

5. **图谱数据**：图谱数据区域位于文件中间（512 到 L-36 字节），具体格式需参考相关规范。

6. **CRC32 校验**：文件尾部包含 CRC32 校验码，使用标准 CRC32 算法计算。

7. **预留字段**：预留字段通常填充 0x00。

## 示例代码

完整的示例代码请参考：
- `ImageData_example.cpp` - 详细使用示例
- `ImageData_test.cpp` - 简单测试程序

## 相关文件

- `ImageData.h` - 图像数据解析类头文件
- `ImageData_example.cpp` - 使用示例代码
- `ImageData_test.cpp` - 测试程序
- `ImageData_README.md` - 本文档

## 参见

- `InfraredSpectrumData.h` - 红外图谱数据解析类
- `CommunicationProtocol.h` - 通信协议定义
