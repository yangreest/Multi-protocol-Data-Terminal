# 红外图谱数据格式解析类

## 概述

`InfraredSpectrumData` 类用于解析 Q/GDW 标准红外图谱数据格式（表 12），该格式严格遵循电力行业标准，用于红外检测设备的图谱数据存储和传输。

## 数据格式说明

红外图谱数据格式包含以下字段（按字节顺序排列）：

### 固定头部（379 字节）

| 序号 | 字段名称 | 数据类型 | 长度 | 字节范围 | 备注 |
|------|----------|----------|------|----------|------|
| 1 | 检测数据类型编码 | uint8 | 1 字节 | [0:0] | 标识图谱类型 |
| 2 | 图谱数据长度 | int32 | 4 字节 | [1:4] | 图谱总长度 |
| 3 | 图谱生成时间 | int64 | 8 字节 | [5:12] | 格式：YYYYMMDDhhmmssfff |
| 4 | 图谱性质 | uint8 | 1 字节 | [13:13] | 0x03:单相，0x04:局部，0x05:三相 |
| 5 | 被检测设备名称 | char | 118 字节 | [14:131] | UNICODE 编码，0x0000 结尾 |
| 6 | 被检测设备编码 | char | 42 字节 | [132:173] | ASCII 编码，\0 结尾 |
| 7 | 测点名称 | char | 128 字节 | [174:301] | UNICODE 编码 |
| 8 | 测点编码 | char | 32 字节 | [302:333] | ASCII 编码 |
| 9 | 检测通道标志 | int16 | 2 字节 | [334:335] | 检测通道标识 |
| 10 | 存储器数据类型 | uint8 | 1 字节 | [336:336] | 存储器类型 |
| 11 | 温度单位 | uint8 | 1 字节 | [337:337] | 温度单位编码 |
| 12 | 温度点阵宽度 | int32 | 4 字节 | [338:341] | 红外图像宽度 |
| 13 | 温度点阵高度 | int32 | 4 字节 | [342:345] | 红外图像高度 |
| 14 | 可见光照片数据长度 | int32 | 4 字节 | [346:349] | L1，无照片时为 0 |
| 15 | 红外照片数据长度 | int32 | 4 字节 | [350:353] | L2，无照片时为 0 |
| 16 | 辐射率 | float | 4 字节 | [354:357] | [0~1.00] |
| 17 | 测试距离 | float | 4 字节 | [358:361] | 单位：m |
| 18 | 大气温度 | float | 4 字节 | [362:365] | 单位：°C |
| 19 | 相对湿度 | int8 | 1 字节 | [366:366] | [0, 100]% |
| 20 | 反射温度 | float | 4 字节 | [367:370] | 单位：°C |
| 21 | 温宽上限 | float | 4 字节 | [371:374] | 单位：温度单位 |
| 22 | 温宽下限 | float | 4 字节 | [375:378] | 单位：温度单位 |

### 可变数据部分

| 序号 | 字段名称 | 长度 | 字节范围 | 备注 |
|------|----------|------|----------|------|
| 23 | 可见光照片数据 | L1 | [379:379+L1-1] | 二进制数据 |
| 24 | 红外照片数据 | L2 | [379+L1:379+L1+L2-1] | 二进制数据 |

## 使用方法

### 1. 从 CommunicationProtocol 解析

```cpp
#include "InfraredSpectrumData.h"
#include "CommunicationProtocol.h"

// 假设已有 CommunicationProtocol 对象
CommunicationProtocol protocol;
// ... 从网络或文件加载数据 ...

// 创建红外图谱数据对象并解析
InfraredSpectrumData spectrumData;
if (spectrumData.parseFromBytes(protocol.detectionFileData))
{
    // 解析成功，访问数据
    std::cout << "设备名称：" << spectrumData.getDeviceNameString() << std::endl;
    std::cout << "测点名称：" << spectrumData.getMeasurementPointNameString() << std::endl;
    std::cout << "温度矩阵：" << spectrumData.temperatureMatrixWidth 
              << " x " << spectrumData.temperatureMatrixHeight << std::endl;
}
```

### 2. 序列化到字节流

```cpp
InfraredSpectrumData spectrumData;

// 填充数据
spectrumData.dataTypeCode = 0x01;
spectrumData.createTime = 20240401153000123LL;
spectrumData.temperatureMatrixWidth = 320;
spectrumData.temperatureMatrixHeight = 240;
spectrumData.emissivity = 0.95f;
// ... 填充其他字段 ...

// 序列化为字节流
std::vector<uint8_t> bytes = spectrumData.toBytes();

// 设置到通信协议中
CommunicationProtocol protocol;
protocol.setDetectionFile(bytes);
```

### 3. 直接解析字节流

```cpp
std::vector<uint8_t> rawData;
// ... 加载原始数据 ...

InfraredSpectrumData spectrumData;
if (spectrumData.parseFromBytes(rawData))
{
    // 访问解析后的数据
}
```

### 4. 保存照片数据

```cpp
// 保存可见光照片
if (!spectrumData.visibleLightPhotoData.empty())
{
    FILE* file = fopen("visible_light.jpg", "wb");
    fwrite(spectrumData.visibleLightPhotoData.data(), 
           1, 
           spectrumData.visibleLightPhotoData.size(), 
           file);
    fclose(file);
}

// 保存红外照片
if (!spectrumData.infraredPhotoData.empty())
{
    FILE* file = fopen("infrared.jpg", "wb");
    fwrite(spectrumData.infraredPhotoData.data(), 
           1, 
           spectrumData.infraredPhotoData.size(), 
           file);
    fclose(file);
}
```

## 主要 API

### 解析方法

- `bool parseFromBytes(const uint8_t* data, size_t length)` - 从字节流解析
- `bool parseFromBytes(const std::vector<uint8_t>& data)` - 从 vector 解析

### 序列化方法

- `std::vector<uint8_t> toBytes() const` - 序列化为字节流

### 辅助方法

- `std::string getDeviceNameString()` - 获取设备名称（UNICODE 解码）
- `std::string getDeviceCodeString()` - 获取设备编码（ASCII 解码）
- `std::string getMeasurementPointNameString()` - 获取测点名称
- `std::string getMeasurementPointCodeString()` - 获取测点编码
- `std::string getCreateTimeString()` - 获取格式化时间字符串
- `std::string getSpectrumPropertyDescription()` - 获取图谱性质描述

## 时间格式说明

时间字段 `createTime` 为 int64 类型，格式为：`YYYYMMDDhhmmssfff`

示例：
- 数值：`20240401153000123`
- 含义：2024 年 04 月 01 日 15 时 30 分 00 秒 123 毫秒
- 格式化输出：`"2024-04-01 15:30:00.123"`

## 图谱性质编码

- `0x03` - 检测设备单相整体图谱
- `0x04` - 检测设备注意相局部图谱
- `0x05` - 检测设备所属三相设备图谱

## 注意事项

1. **字节序**：所有多字节整数类型均采用大端模式（Big-Endian）
2. **字符串编码**：
   - 设备名称、测点名称使用 UNICODE 编码（UTF-16 BE）
   - 设备编码、测点编码使用 ASCII 编码
3. **最小数据长度**：不含照片数据时，最小长度为 379 字节
4. **照片数据**：如无照片，对应长度字段填 0，不存储照片数据

## 示例代码

完整示例代码请参考 `InfraredSpectrumData_example.cpp`

## 相关文件

- `InfraredSpectrumData.h` - 红外图谱数据解析类定义
- `InfraredSpectrumData_example.cpp` - 使用示例代码
- `CommunicationProtocol.h` - 通信协议类（包含 detectionFileData 字段）
