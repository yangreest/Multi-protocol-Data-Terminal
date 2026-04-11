#include "serial_protocol_parser.h"
#include <algorithm>
#include <iostream>
#include <qdebug.h>

// MultiPacketCache 成员方法实现
bool MultiPacketCache::is_complete() const {
	return received_packets.size() == total_packets;
}

// 单包合法性校验
bool SerialProtocolParser::validate_single_packet(const std::vector<uint8_t>& packet) {
	// 1. 最小长度校验：包头(1)+包长度(2)+总包数(1)+包计数(1)+包尾(1) = 7字节
	if (packet.size() < 7) return false;
	// 2. 包头包尾校验
	if (packet[0] != PACKET_HEADER || packet.back() != PACKET_TAIL) return false;
	// 3. 包长度校验（小端模式，高位在后）
	uint16_t packet_len = packet[1] << 8 | (packet[2]);
	if (packet_len > MAX_PACKET_LENGTH || packet_len != packet.size()) return false;
	// 4. 包计数合法性
	uint8_t total = packet[3];
	uint8_t count = packet[4];
	if (count < 1 || count > total) return false;

	return true;
}

// 提取单包有效数据
std::vector<uint8_t> SerialProtocolParser::extract_packet_data(const std::vector<uint8_t>& packet) {
	// 数据段：包计数(索引4)之后，包尾(最后1字节)之前
	size_t data_start = 5;
	size_t data_end = packet.size() - 1;
	std::vector<uint8_t> packet_data = std::vector<uint8_t>(packet.begin() + data_start, packet.begin() + data_end);
	return packet_data;
}

// 核心解析方法实现
std::vector<uint8_t> SerialProtocolParser::parse_serial_data(const std::vector<uint8_t>& new_data) {
	// 1. 追加新数据到缓冲区
	receive_buffer.insert(receive_buffer.end(), new_data.begin(), new_data.end());

	// 2. 循环解析缓冲区中的完整包
	std::vector<uint8_t> complete_data;
	while (true) {
		// 查找包头位置
		auto header_it = std::find(receive_buffer.begin(), receive_buffer.end(), PACKET_HEADER);
		if (header_it == receive_buffer.end()) break;

		// 查找包尾位置
		auto tail_it = std::find(header_it, receive_buffer.end(), PACKET_TAIL);
		if (tail_it == receive_buffer.end()) break;

		// 提取完整包并移除缓冲区中的对应数据
		std::vector<uint8_t> single_packet(header_it, tail_it + 1);
		receive_buffer.erase(receive_buffer.begin(), tail_it + 1);

		// 3. 单包校验
		if (!validate_single_packet(single_packet)) {
			qDebug() << "[SerialParser] 单包校验失败，丢弃异常包";
			continue;
		}

		// 4. 解析核心字段
		uint8_t total_packets = single_packet[3];  // 总包数
		uint8_t packet_count = single_packet[4];   // 包计数
		uint16_t packet_len = (single_packet[1] << 8) | single_packet[2]; // 包长度

		// 5. 处理响应包（固定长度7字节）
		if (packet_len == RESPONSE_PACKET_LEN) {
			uint8_t result = single_packet[5]; // 0=成功，1=失败
			qDebug() << "[SerialParser] 收到响应包：" << (result == 0 ? "接收成功" : "接收失败");
			complete_data.push_back(result);
			continue;
		}

		// 6. 多包拼接逻辑（cache_key 可根据实际需求扩展为更唯一的标识）
		uint32_t cache_key = total_packets;
		auto& cache = multi_packet_cache[cache_key];
		cache.total_packets = total_packets;

		// 按包计数存储数据（索引从1开始）
		if (packet_count < cache.received_packets.size()) {
			cache.received_packets[packet_count - 1] = extract_packet_data(single_packet);
		}
		else {
			cache.received_packets.resize(packet_count);
			cache.received_packets[packet_count - 1] = extract_packet_data(single_packet);
		}

		// 7. 检查是否所有包接收完成，完成则拼接
		if (cache.is_complete()) {
			for (const auto& pkt_data : cache.received_packets) {
				complete_data.insert(complete_data.end(), pkt_data.begin(), pkt_data.end());
			}
			multi_packet_cache.erase(cache_key); // 清空缓存
			qDebug() << "[SerialParser] 多包拼接完成，总长度：" << complete_data.size();
			packet_assembled_callback_(complete_data);
		}
	}

	return complete_data;
}

std::vector<std::vector<uint8_t>> SerialProtocolParser::split_long_data(const std::vector<uint8_t>& long_data, int chunk_size)
{
	std::vector<std::vector<uint8_t>> chunks;
	int tolal_chunks = long_data.size() / chunk_size + 1;
	for (size_t i = 0; i < long_data.size(); i += chunk_size) {
		std::vector<uint8_t> chunk;
		chunk.push_back(PACKET_HEADER);
		// 包长度 两个字节
		//chunk_size 并不是实际的长度，有可能最后一个块的长度不足chunk_size，那么下方的chunk_size + 6可能大于实际的长度
		if (i + chunk_size > long_data.size())
		{
			chunk_size = long_data.size() - i;
		}

		chunk.push_back(static_cast<uint8_t>(((chunk_size + 6) >> 8) & 0xFF));
		chunk.push_back(static_cast<uint8_t>((chunk_size + 6) & 0xFF));
		chunk.push_back(static_cast<uint8_t>(tolal_chunks));
		chunk.push_back(static_cast<uint8_t>(i / chunk_size + 1));
		// 此处填入数据
		chunk.insert(chunk.end(), long_data.begin() + i, long_data.begin() + std::min(i + chunk_size, long_data.size()));
		chunk.push_back(PACKET_TAIL);
		chunks.push_back(chunk);
	}
	return chunks;
}

// 清空缓冲区和缓存
void SerialProtocolParser::clear() {
	receive_buffer.clear();
	multi_packet_cache.clear();
}