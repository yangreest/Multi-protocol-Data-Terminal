#include "DataTerminalProgram.h"
#include "CommunicationProtocol.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

DataTerminalProgram::DataTerminalProgram(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

DataTerminalProgram::~DataTerminalProgram()
{
}

void DataTerminalProgram::on_pushButton_2_clicked()
{
	m_pComDevice = new CTcpClientCom();

	QString IP = ui.lineEdit->text();
    bool ok;
	int nPort = ui.lineEdit_2->text().toInt(&ok);
    if (!ok)
    {
        qDebug() << "Invalid port number";
        return;
    }

	m_pComDevice->SetParam(IP.toStdString().c_str(), nPort);
	m_pComDevice->RegisterReadDataCallBack(std::bind(&DataTerminalProgram::ReceiveNewData, this, std::placeholders::_1, std::placeholders::_2));
	m_pComDevice->RegisterConnectStatusCallBack(std::bind(&DataTerminalProgram::ComDeviceConnectionChanged, this, std::placeholders::_1, std::placeholders::_2, 0));
	m_pComDevice->BeginWork();
}

QString DataTerminalProgram::formatXmlString(const std::string& str)
{
	QXmlStreamReader reader(str);
	QString result;

	// 写入器：自动格式化、缩进、换行
	QXmlStreamWriter writer(&result);
	writer.setAutoFormatting(true);      // 开启自动格式化
	writer.setAutoFormattingIndent(2);   // 缩进2个空格（美观）

	// 逐节点读取并写入，自动修复语法
	while (!reader.atEnd()) {
		reader.readNext();
		// 跳过错误节点（容错处理）
		if (reader.hasError()) {
			reader.readNext();
			continue;
		}
		writer.writeCurrentToken(reader);
	}

	return result;
}

QString DataTerminalProgram::getPacketTypeName(uint32_t packetTypeCode)
{
	if (packetTypeCode == 0x00000001)
	{
        return "任务下发";
	}
    if (packetTypeCode == 0x00000002)
	{
        return "请求连接";
	}
    if (packetTypeCode == 0x00000003)
	{
        return "检测数据文件上传";
	}
    if (packetTypeCode == 0x80000001)
	{
        return "任务接收确认";
	}
    if (packetTypeCode == 0x80000002)
	{
        return "请求连接确认";
	}
    if (packetTypeCode == 0x80000003)
	{
        return "检测数据文件上传确认";
	}
	return QString();
}

void DataTerminalProgram::ReceiveNewData(const uint8_t* p, int len)
{
}

void DataTerminalProgram::ComDeviceConnectionChanged(const bool connected, int guid, int index)
{
    if (connected)
    {
		ui.pushButton_2->setEnabled(false);
		ui.pushButton_2->setText("已连接");
        qDebug() << "设备已连接";
    }
    else
    {
        ui.pushButton_2->setEnabled(true);
        ui.pushButton_2->setText("掉线请重试");
        qDebug() << "设备已断开";
    }
}

void DataTerminalProgram::on_pushButton_clicked()
{
	QString strData = ui.textEdit->toPlainText();
	// 使用您提供的十六进制字符串
	//std::string userHexString = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";

	CommunicationProtocol protocol;
	if (protocol.buildFromHexString(strData.toStdString()))
	{
		qDebug() << "✓ 成功解析用户提供的十六进制字符串";
		qDebug() << "报文头：0x" << std::hex << protocol.packetHeader;
		qDebug() << "版本号：" << (int)protocol.version;
		qDebug() << "序号：" << protocol.sequenceNumber;
		qDebug() << "请求标志：0x" << (int)protocol.requestFlag;
		qDebug() << "总长度：" << protocol.totalPacketLength;
		qDebug() << "报文类型：0x" << protocol.packetTypeCode;
		qDebug() << "压缩标志：0x" << (int)protocol.compressionFlag;
		qDebug() << "加密标志：0x" << (int)protocol.encryptionFlag;
		qDebug() << "仪器厂商：0x" << (int)protocol.instrumentVendor;
		qDebug() << "业务数据格式：0x" << (int)protocol.serviceDataFormat;
		qDebug() << "业务数据长度：" << protocol.serviceDataLength;
		qDebug() << "业务内容：" << protocol.getServiceDataString();
		qDebug() << "检测文件长度：" << protocol.detectionFileLength;
		qDebug() << "CRC32: 0x" << protocol.crc32Checksum;
		qDebug() << "报文尾：0x" << (int)protocol.packetTail;

		ui.plainTextEdit->appendPlainText(QString("报文类型：%1;编码：%2").arg(getPacketTypeName(protocol.packetTypeCode)).arg(QString::number(protocol.packetTypeCode, 16)));
		std::string standardXml = protocol.getServiceDataString();
		// 如果plainTextEdit是QPlainTextEdit类型，可以直接追加
		ui.plainTextEdit->appendPlainText(formatXmlString(standardXml));
		ui.plainTextEdit->setFont(QFont("Consolas", 10)); // 等宽字体，显示更美观

		// 验证 CRC
		if (protocol.verifyCRC32())
		{
			qDebug() << "✓ CRC32 校验通过";
		}
		else
		{
			qDebug() << "⚠ CRC32 校验失败（可能是示例字符串的 CRC 值不匹配）";
		}
	}
	else
	{
		qDebug() << "✗ 解析失败";
	}
}

