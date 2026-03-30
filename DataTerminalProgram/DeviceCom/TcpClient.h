#pragma once
#include <condition_variable>
#include <queue>
#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <winsock2.h>

// 缓冲区大小优化
constexpr size_t RECEIVE_BUFFER_SIZE = 65536;  // 64KB 接收缓冲区
constexpr size_t SEND_BUFFER_SIZE = 65536;     // 64KB 发送缓冲区
constexpr size_t MAX_PENDING_BYTES = 10485760; // 10MB 最大待发送数据

class CTcpClientCom
{
public:
	CTcpClientCom();
	~CTcpClientCom();
	void SetParam(const char* pComName, int nComPort);
	void RegisterReadDataCallBack(const std::function<void(uint8_t*, int, uint64_t)>& f);
	void RegisterConnectStatusCallBack(const std::function<void(bool, int)>& f);
	bool Write(uint8_t* data, size_t len);
	bool BeginWork();
	bool EndWork();
	
	// 获取统计信息
	struct Statistics {
		uint64_t totalBytesSent;
		uint64_t totalBytesReceived;
		size_t currentPendingBytes;
		uint64_t sendCount;
		uint64_t receiveCount;
	};
	Statistics GetStatistics() const;

private:
	void ConnectionThread();
	void Connect();
	void CloseSocket();
	void SetConnected(bool b);
	void SendThread();
	void ReceiveThread();
	
	// 智能指针管理缓冲区
	using DataBuffer = std::shared_ptr<std::vector<uint8_t>>;

	std::string m_strTargetIp;
	uint16_t m_wTargetPort;
	SOCKET m_socket;

	std::atomic<bool> m_running;
	std::atomic<bool> m_connected;
	std::atomic<uint64_t> m_totalBytesSent;
	std::atomic<uint64_t> m_totalBytesReceived;
	std::atomic<size_t> m_currentPendingBytes;
	std::atomic<uint64_t> m_sendCount;
	std::atomic<uint64_t> m_receiveCount;

	std::thread m_connectionThread;
	std::thread m_receiveThread;
	std::thread m_sendThread;

	// 双队列设计，减少锁竞争
	std::queue<DataBuffer> m_sendQueue;
	std::mutex m_sendQueueMutex;
	std::condition_variable m_sendQueueCV;
	
	// 背压信号
	std::atomic<bool> m_backPressure;

	std::function<void(bool, int)> m_connectCallback;
	std::function<void(uint8_t*, int, uint64_t)> m_function_ReadDataCallBack;
};
