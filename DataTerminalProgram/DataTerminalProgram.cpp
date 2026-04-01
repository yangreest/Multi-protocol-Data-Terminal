#include "DataTerminalProgram.h"
#include "CommunicationProtocol.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QTimer>

DataTerminalProgram::DataTerminalProgram(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);


	QStringList portList = enumerateSerialPorts();
	ui.comboBox->addItems(portList);

	// 初始化队列和工作线程
	m_sendQueue = new SerialDataQueue();
	m_receiveQueue = new SerialDataQueue();
	m_serialWorker = nullptr;

	// 启动接收数据处理定时器
	QTimer* receiveTimer = new QTimer(this);
	connect(receiveTimer, &QTimer::timeout, this, &DataTerminalProgram::processReceivedData);
	receiveTimer->start(50); // 每50ms检查一次接收数据
}

DataTerminalProgram::~DataTerminalProgram()
{
}

void DataTerminalProgram::on_pushButton_2_clicked()
{
	if (ui.pushButton_2->text() == "连接") {
		openSerialPort();
	}
	else {
		closeSerialPort();
		ui.pushButton_2->setText("连接");
		//on_btnClear_clicked();
	}
}

void DataTerminalProgram::on_pushButton_5_clicked()
{
}

void DataTerminalProgram::onSerialError(const QString& error)
{
	qDebug() <<"onSerialError :"<< error;
}

void DataTerminalProgram::onSerialPortOpened()
{
    qDebug() << "串口已打开";
}

void DataTerminalProgram::onSerialPortClosed()
{
    qDebug() << "串口已关闭";
}

// 枚举可用串口的函数
QStringList DataTerminalProgram::enumerateSerialPorts()
{
	QStringList portList;

#ifdef Q_OS_WIN
	// Windows下通过查询注册表或试探法获取串口列表
	for (int i = 1; i <= 10; i++) {
		QString portName = QString("COM%1").arg(i);
		QextSerialPort port(portName, QextSerialPort::Polling);
		// 尝试打开端口来检测是否存在
		if (port.open(QIODevice::ReadWrite)) {
			portList.append(portName);
			port.close();
			port.deleteLater();
		}
		else {
			// 忽略打开失败的端口
			continue;
		}
	}
#else
	// Linux/macOS下扫描设备文件
	QDir dir("/dev");
	QStringList nameFilters;
	nameFilters << "ttyS*" << "ttyUSB*" << "ttyACM*" << "cu.*" << "tty.*";

	QFileInfoList fileInfos = dir.entryInfoList(nameFilters, QDir::Files | QDir::System);
	foreach(const QFileInfo & fileInfo, fileInfos) {
		QString fileName = fileInfo.fileName();
		if (!fileName.startsWith("ttyprintk")) { // 排除内核打印端口
			portList.append(fileInfo.absoluteFilePath());
		}
	}
#endif

	return portList;
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

void DataTerminalProgram::processReceivedData()
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

void DataTerminalProgram::openSerialPort()
{
	if (m_serialWorker && m_serialWorker->isRunning()) {
		return;
	}

	QString portName = ui.comboBox->currentText();

	m_serialWorker = new SerialWorker(m_sendQueue, m_receiveQueue);
	m_serialWorker->setPortSettings(portName);

	connect(m_serialWorker, &SerialWorker::errorOccurred,
		this, &DataTerminalProgram::onSerialError);
	connect(m_serialWorker, &SerialWorker::portOpened,
		this, &DataTerminalProgram::onSerialPortOpened);
	connect(m_serialWorker, &SerialWorker::portClosed,
		this, &DataTerminalProgram::onSerialPortClosed);

	m_serialWorker->start();
}

void DataTerminalProgram::closeSerialPort()
{
	if (m_serialWorker) {
		m_serialWorker->stop();
		m_serialWorker->wait();
		m_serialWorker->deleteLater();
		m_serialWorker = nullptr;
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

