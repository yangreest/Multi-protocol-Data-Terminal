#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DataTerminalProgram.h"
#include "CommunicationProtocol.h"
#include "DeviceCom/serialdataqueue.h"
#include "DeviceCom/serialworker.h"

class DataTerminalProgram : public QMainWindow
{
	Q_OBJECT

public:
	DataTerminalProgram(QWidget* parent = nullptr);
	~DataTerminalProgram();

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_5_clicked();

	void onSerialError(const QString& error);
	void onSerialPortOpened();
	void onSerialPortClosed();

private:
	QString formatXmlString(const std::string& str);
	// 报文类型
	QString getPacketTypeName(uint32_t packetTypeCode);

	void processReceivedData();
	void ComDeviceConnectionChanged(const bool connected, int guid, int index);

	void openSerialPort();
    void closeSerialPort();
	QStringList enumerateSerialPorts();

private:
	Ui::DataTerminalProgramClass ui;

private:
	SerialDataQueue* m_sendQueue;
	SerialDataQueue* m_receiveQueue;
	SerialWorker* m_serialWorker;
};

