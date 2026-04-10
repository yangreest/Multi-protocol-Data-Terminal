#ifndef SERIAL_PROTOCOL_PARSER_H
#define SERIAL_PROTOCOL_PARSER_H

#include <vector>
#include <unordered_map>
#include <cstdint>

// 协议常量定义（对外可见）
const uint8_t PACKET_HEADER = 0xA5;    // 包头
const uint8_t PACKET_TAIL = 0xFE;      // 包尾
const uint16_t MAX_PACKET_LENGTH = 507;// 最大包长度
const uint8_t RESPONSE_PACKET_LEN = 7; // 成功/失败响应包固定长度

// 多包数据缓存结构（内部使用，对外声明为私有）
struct MultiPacketCache {
    uint8_t total_packets;             // 总包数
    std::vector<std::vector<uint8_t>> received_packets; // 已接收的包数据
    bool is_complete() const;          // 判断是否接收完整
};

// 串口协议解析类（对外核心类）
class SerialProtocolParser {
private:
    std::unordered_map<uint32_t, MultiPacketCache> multi_packet_cache; // 多包缓存
    std::vector<uint8_t> receive_buffer; // 串口接收缓冲区

    // 单包合法性校验（内部方法）
    bool validate_single_packet(const std::vector<uint8_t>& packet);
    // 提取单包有效数据（内部方法）
    std::vector<uint8_t> extract_packet_data(const std::vector<uint8_t>& packet);

public:
    // 接收串口数据并解析（核心对外接口）
    // 返回值：拼接完成的完整数据（响应包返回0/1，透传包返回拼接后的原始数据）
    std::vector<uint8_t> parse_serial_data(const std::vector<uint8_t>& new_data);

    // 清空缓冲区和缓存（对外接口）
    void clear();

    // 析构函数
    ~SerialProtocolParser() = default;
};

#endif // SERIAL_PROTOCOL_PARSER_H