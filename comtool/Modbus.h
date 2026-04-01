#include <vector>
#ifndef MODBUS_H
#define MODBUS_H
class Modbus {

public:
	// -------------------------- 1. CRC16校验函数（严格遵循附录B） --------------------------
	// 输入：数据缓冲区、数据长度；输出：2字节CRC校验值（高低字节已交换，符合协议要求）
	static uint16_t CRC16_Modbus(const uint8_t* buf, uint16_t len) {
		uint16_t crc = 0xFFFF;  // 初始值
		for (uint16_t i = 0; i < len; i++) {
			crc ^= (uint16_t)buf[i];  // 逐字节异或
			for (uint8_t j = 0; j < 8; j++) {  // 8位循环移位
				if ((crc & 0x0001) != 0) {     // 最低位为1时
					crc >>= 1;
					crc ^= 0xA001;             // 异或多项式
				}
				else {
					crc >>= 1;
				}
			}
		}
		// 高低字节交换（协议要求CRC校验码低字节在前、高字节在后）
		crc = ((crc & 0x00FF) << 8) | ((crc & 0xFF00) >> 8);
		return crc;
	}

	// 读取站点
	static std::vector<uint8_t> Modbus_Read_Station() {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x03);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));      // 第8字节：CRC高8位
		buf.push_back((uint8_t)(crc & 0x00FF));  // 第7字节：CRC低8位
		return buf;
	}


	// 读取波特率 {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA};  //读取波特率
	static std::vector<uint8_t> Modbus_Read_BT() {

		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x03);
		buf.push_back(0x00);
		buf.push_back(0x01);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}

	// 读取触发{0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA} 
	static std::vector<uint8_t> Modbus_Read_Trigger() {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x03);
		buf.push_back(0x00);
		buf.push_back(0x02);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}

	// 读取触发{0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA} 
	static std::vector<uint8_t> Modbus_Set_Trigger(uint8_t data) {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x06);
		buf.push_back(0x00);
		buf.push_back(0x02);
		buf.push_back(0x00);
		buf.push_back(data);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}


	// 读取检测状态 {0x01, 0x04, 0x00, 0x10, 0x00, 0x01, 0x30, 0x0F}
	static std::vector<uint8_t> Modbus_Read_Detection_State() {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x04);
		buf.push_back(0x00);
		buf.push_back(0x10);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}

	// 读取检测结果 {0x01, 0x04, 0x00, 0x11, 0x00, 0x01, 0x61, 0xCF}; 
	static std::vector<uint8_t> Modbus_Read_Detection_Result() {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x04);
		buf.push_back(0x00);
		buf.push_back(0x11);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}

	// 读取电量 {0x01, 0x04, 0x00, 0x12, 0x00, 0x01, 0x91, 0xCF}; 
	static std::vector<uint8_t> Modbus_Read_Power() {
		std::vector<uint8_t> buf;
		buf.push_back(0x01);
		buf.push_back(0x04);
		buf.push_back(0x00);
		buf.push_back(0x12);
		buf.push_back(0x00);
		buf.push_back(0x01);
		// 2. 计算CRC校验（对前6字节计算）
		uint16_t crc = CRC16_Modbus(buf.data(), buf.size());
		// 3. 填充CRC（低字节在前，高字节在后）
		buf.push_back((uint8_t)(crc >> 8));
		buf.push_back((uint8_t)(crc & 0x00FF));
		return buf;
	}

	//校验回包
	static bool Modbus_Check_Response(QByteArray buf, uint16_t& data) {
		// 计算CRC校验
		uint16_t crc = CRC16_Modbus((uint8_t*)buf.data(), buf.size() - 2);
		if (buf[buf.size() - 2] == (crc >> 8) && buf[buf.size() - 1] == (crc & 0x00FF))
		{

		}
		else
		{
            return false;
		}
	};

};
#endif // !
