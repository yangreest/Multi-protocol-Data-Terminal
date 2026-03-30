#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DataTerminalProgram.h"
#include "CommunicationProtocol.h"
#include "DeviceCom/TcpClient.h"

class DataTerminalProgram : public QMainWindow
{
	Q_OBJECT

public:
	DataTerminalProgram(QWidget* parent = nullptr);
	~DataTerminalProgram();

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	

private:
	QString formatXmlString(const std::string& str);
	// 报文类型
	QString getPacketTypeName(uint32_t packetTypeCode);

	void ReceiveNewData(const uint8_t* p, int len);
	void ComDeviceConnectionChanged(const bool connected, int guid, int index);

private:
	Ui::DataTerminalProgramClass ui;

	CTcpClientCom* m_pComDevice;
};

