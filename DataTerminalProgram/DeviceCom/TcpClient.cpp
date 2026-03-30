#include "TcpClient.h"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <memory>

#ifdef __DEBUG__
bool bWriteLog = true;
#include <iostream>
#include <sstream>
#include <iomanip>

#endif
#include <qlogging.h>
#include <QDebug>

#pragma comment(lib, "ws2_32.lib")

CTcpClientCom::CTcpClientCom() : m_wTargetPort(0), m_socket(INVALID_SOCKET)
{
	m_function_ReadDataCallBack = nullptr;
	m_connectCallback = nullptr;
	m_running = false;
	m_connected = false;
	m_totalBytesSent = 0;
	m_totalBytesReceived = 0;
	m_currentPendingBytes = 0;
	m_sendCount = 0;
	m_receiveCount = 0;
	m_backPressure = false;
}

CTcpClientCom::~CTcpClientCom()
{
	EndWork();
}

void CTcpClientCom::SetParam(const char* pComName, int nComPort)
{
	m_strTargetIp = pComName;
	m_wTargetPort = nComPort;
}

void CTcpClientCom::RegisterReadDataCallBack(const std::function<void(uint8_t*, int, uint64_t)>& f)
{
	m_function_ReadDataCallBack = f;
}

void CTcpClientCom::RegisterConnectStatusCallBack(const std::function<void(bool, int)>& f)
{
	m_connectCallback = f;
}

bool CTcpClientCom::Write(uint8_t* data, size_t len)
{
	if (!m_running || !m_connected)
	{
		return false;
	}

	// 背压检查：如果待发送数据超过阈值，拒绝写入
	if (m_currentPendingBytes.load() >= MAX_PENDING_BYTES)
	{
		qWarning() << "背压触发：待发送数据已达上限" << m_currentPendingBytes.load();
		return false;
	}

	// 使用智能指针管理数据，避免拷贝
	auto buffer = std::make_shared<std::vector<uint8_t>>(data, data + len);
	
	{
		std::unique_lock<std::mutex> lock(m_sendQueueMutex);
		m_currentPendingBytes += len;
		m_sendQueue.push(buffer);
		m_sendQueueCV.notify_one();
	}

	return true;
}

bool CTcpClientCom::BeginWork()
{
	if (m_running)
	{
		return true;
	}

	// 初始化 Winsock
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		qCritical() << "WSAStartup 失败：" << ret;
		return false;
	}

	m_running = true;
	m_connected = false;

	// 启动连接线程
	m_connectionThread = std::thread(&CTcpClientCom::ConnectionThread, this);

	// 启动接收线程
	m_receiveThread = std::thread(&CTcpClientCom::ReceiveThread, this);

	//启动发送线程
	m_sendThread = std::thread(&CTcpClientCom::SendThread, this);

	return true;
}

bool CTcpClientCom::EndWork()
{
	if (!m_running)
	{
		return true;
	}

	m_running = false;

	// 通知条件变量
	{
		std::unique_lock<std::mutex> lock(m_sendQueueMutex);
		m_sendQueueCV.notify_all();
	}

	// 关闭套接字
	CloseSocket();

	// 等待线程结束
	if (m_connectionThread.joinable())
	{
		m_connectionThread.join();
	}

	if (m_receiveThread.joinable())
	{
		m_receiveThread.join();
	}

	if (m_sendThread.joinable())
	{
		m_sendThread.join();
	}

	// 清理 Winsock
	WSACleanup();
	return true;
}

CTcpClientCom::Statistics CTcpClientCom::GetStatistics() const
{
	Statistics stats;
	stats.totalBytesSent = m_totalBytesSent.load();
	stats.totalBytesReceived = m_totalBytesReceived.load();
	stats.currentPendingBytes = m_currentPendingBytes.load();
	stats.sendCount = m_sendCount.load();
	stats.receiveCount = m_receiveCount.load();
	return stats;
}

void CTcpClientCom::ConnectionThread()
{
	while (m_running)
	{
		if (!m_connected)
		{
			Connect();
		}

		// 等待一段时间再尝试重连
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

// 设置连接状态
void CTcpClientCom::SetConnected(bool connected)
{
	if (m_connected != connected)
	{
		m_connected = connected;

		// 调用连接状态回调
		if (m_connectCallback != nullptr)
		{
			m_connectCallback(connected, 0);
		}
		
		if (!connected)
		{
			// 断开连接时清空发送队列
			std::unique_lock<std::mutex> lock(m_sendQueueMutex);
			while (!m_sendQueue.empty()) {
				m_sendQueue.pop();
			}
			m_currentPendingBytes = 0;
		}
	}
}

void CTcpClientCom::CloseSocket()
{
	if (m_socket != INVALID_SOCKET)
	{
		// 优雅关闭
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	SetConnected(false);
}

void CTcpClientCom::Connect()
{
	CloseSocket();

	// 创建套接字
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		qCritical() << "创建套接字失败：" << WSAGetLastError();
		return;
	}

	// 优化 TCP 参数
	int optval;
	socklen_t optlen;

	// 增大发送缓冲区
	optval = SEND_BUFFER_SIZE;
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval));

	// 增大接收缓冲区
	optval = RECEIVE_BUFFER_SIZE;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval));

	// 禁用 Nagle 算法以提高实时性（如果需要）
	// optval = 1;
	// setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

	// 设置为非阻塞模式
	unsigned long nonBlocking = 1;
	if (ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0)
	{
		qCritical() << "设置非阻塞模式失败：" << WSAGetLastError();
		CloseSocket();
		return;
	}

	// 设置服务器地址
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(m_wTargetPort);
	memset(&serverAddr, 0, sizeof(serverAddr));

	// 转换 IP 地址 - 修复返回值检查
	int addrRet = inet_pton(AF_INET, m_strTargetIp.c_str(), &serverAddr.sin_addr);
	if (addrRet != 1)
	{
		qCritical() << "IP 地址转换失败：" << addrRet;
		CloseSocket();
		return;
	}

	// 尝试连接
	int result = connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
		{
			// 连接正在进行中，使用 select 检查连接状态
			fd_set writeSet;
			timeval timeout;

			FD_ZERO(&writeSet);
			FD_SET(m_socket, &writeSet);
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			result = select(0, nullptr, &writeSet, nullptr, &timeout);
			if (result > 0)
			{
				if (FD_ISSET(m_socket, &writeSet))
				{
					// 检查是否有错误
					int errorCode;
					int errorSize = sizeof(errorCode);
					if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &errorSize) == 0)
					{
						if (errorCode == 0)
						{
							qInfo() << "TCP 连接成功：" << m_strTargetIp.c_str() << ":" << m_wTargetPort;
							SetConnected(true);
							return;
						}
						else
						{
							qCritical() << "连接错误：" << errorCode;
						}
					}
				}
			}
			else if (result == 0)
			{
				qWarning() << "连接超时";
			}
			else
			{
				qCritical() << "select 失败：" << WSAGetLastError();
			}
		}
		else
		{
			qCritical() << "connect 失败：" << error;
		}
	}
	else
	{
		qInfo() << "TCP 连接成功：" << m_strTargetIp.c_str() << ":" << m_wTargetPort;
		SetConnected(true);
	}

	CloseSocket();
}

void CTcpClientCom::SendThread()
{
	while (m_running)
	{
		if (!m_connected)
		{
			Sleep(10);
			continue;
		}
		
		DataBuffer buffer;

		// 等待数据或连接关闭
		{
			std::unique_lock<std::mutex> lock(m_sendQueueMutex);
			m_sendQueueCV.wait(lock, [this] { 
				return !m_sendQueue.empty() || !m_connected || !m_running; 
			});
			
			if (!m_connected || !m_running || m_sendQueue.empty())
			{
				continue;
			}

			buffer = m_sendQueue.front();
			m_sendQueue.pop();
		}

		// 发送数据 - 零拷贝优化
		if (buffer && !buffer->empty() && m_connected)
		{
			size_t totalSent = 0;
			size_t remaining = buffer->size();
			
			while (totalSent < remaining && m_connected && m_running)
			{
				auto bytesToSend = static_cast<int>(std::min(remaining - totalSent, 
					static_cast<size_t>(SEND_BUFFER_SIZE)));
					
				auto bytesSent = send(m_socket, 
					reinterpret_cast<const char*>(buffer->data() + totalSent), 
					bytesToSend, 0);
					
				if (bytesSent == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK && error != WSAEINTR)
					{
						qCritical() << "发送错误：" << error;
						SetConnected(false);
						break;
					}
					// WSAEWOULDBLOCK: 稍后重试
					Sleep(1);
					continue;
				}
				
				totalSent += bytesSent;
				m_totalBytesSent += bytesSent;
			}
			
			m_sendCount++;
			
			// 更新待发送字节数
			m_currentPendingBytes -= buffer->size();
			
#ifdef __DEBUG__
			if (bWriteLog && totalSent > 0)
			{
				auto toHexString = [](const uint8_t* t, size_t len)
					{
						std::ostringstream oss;
						oss << std::hex << std::setfill('0');
						size_t printLen = std::min(len, size_t(256)); // 只打印前 256 字节
						for (size_t i = 0; i < printLen; ++i)
						{
							oss << std::setw(2) << static_cast<int>(t[i]);
							if (i < printLen - 1) oss << ' ';
						}
						if (len > printLen) oss << "... (" << len << " bytes)";
						return oss.str();
					};
				qDebug() << "发送数据：" << toHexString(buffer->data(), buffer->size()) 
					<< "总发送:" << totalSent << "bytes";
			}
#endif
			
			if (totalSent != buffer->size())
			{
				qWarning() << "发送不完整：期望" << buffer->size() << "实际" << totalSent;
			}
		}
		else
		{
			Sleep(10);
		}
	}
}

// 接收线程函数
void CTcpClientCom::ReceiveThread()
{
	// 分配大缓冲区
	std::vector<char> receiveBuffer(RECEIVE_BUFFER_SIZE);
	
	while (m_running)
	{
		if (m_connected)
		{
			fd_set readSet;
			timeval timeout;

			FD_ZERO(&readSet);
			FD_SET(m_socket, &readSet);
			timeout.tv_sec = 1; // 1 秒超时
			timeout.tv_usec = 0;

			// 使用 select 检查是否有数据可读
			int result = select(0, &readSet, nullptr, nullptr, &timeout);
			if (result > 0)
			{
				if (FD_ISSET(m_socket, &readSet))
				{
					int bytesReceived = recv(m_socket, receiveBuffer.data(), 
						static_cast<int>(receiveBuffer.size()), 0);
						
					if (bytesReceived > 0)
					{
						m_receiveCount++;
						m_totalBytesReceived += bytesReceived;
						
						if (m_function_ReadDataCallBack != nullptr)
						{
							m_function_ReadDataCallBack(
								reinterpret_cast<uint8_t*>(receiveBuffer.data()), 
								bytesReceived, 0);
						}

#ifdef __DEBUG__
						if (bWriteLog)
						{
							auto toHexString = [](const uint8_t* t, int len)
								{
									std::ostringstream oss;
									oss << std::hex << std::setfill('0');
									size_t printLen = std::min(static_cast<size_t>(len), size_t(256));
									for (int i = 0; i < static_cast<int>(printLen); ++i)
									{
										oss << std::setw(2) << static_cast<int>(t[i]);
										if (i < static_cast<int>(printLen) - 1) oss << ' ';
									}
									if (len > static_cast<int>(printLen)) 
										oss << "... (" << len << " bytes)";
									return oss.str();
								};
							qDebug() << "接收数据：" << toHexString(reinterpret_cast<uint8_t*>(receiveBuffer.data()), 
								bytesReceived) << "总接收:" << m_totalBytesReceived.load() << "bytes";
						}
#endif
					}
					else if (bytesReceived == 0)
					{
						// 连接关闭
						qInfo() << "连接已关闭";
						SetConnected(false);
					}
					else
					{
						// 接收错误
						int error = WSAGetLastError();
						if (error != WSAEWOULDBLOCK && error != WSAEINTR)
						{
							qCritical() << "接收错误：" << error;
							SetConnected(false);
						}
					}
				}
			}
			else if (result == 0)
			{
				// 超时，继续循环
			}
			else
			{
				// select 错误
				int error = WSAGetLastError();
				qCritical() << "select 错误：" << error;
				SetConnected(false);
			}
		}
		else
		{
			// 未连接，等待
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
