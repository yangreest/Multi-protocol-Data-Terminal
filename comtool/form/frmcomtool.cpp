#include "frmcomtool.h"
#include "ui_frmcomtool.h"
#include "qthelper.h"
#include "qthelperdata.h"
#include <CommunicationProtocol.h>
#include <InfraredSpectrumData.h>
#include <ImageData.h>


frmComTool::frmComTool(QWidget* parent) : QWidget(parent), ui(new Ui::frmComTool)
{
	ui->setupUi(this);
	this->initForm();
	this->initConfig();
	openSerialPort();

	QtHelper::setFormInCenter(this);
}

frmComTool::~frmComTool()
{
	closeSerialPort();

	delete m_sendQueue;
	delete m_receiveQueue;
	delete ui;
}



void frmComTool::initForm()
{
	sleepTime = 10;
	receiveCount = 0;
	sendCount = 0;
	isShow = true;

	// 初始化队列和工作线程
	m_sendQueue = new SerialDataQueue();
	m_receiveQueue = new SerialDataQueue();
	m_serialWorker = nullptr;

	// 启动接收数据处理定时器
	QTimer* receiveTimer = new QTimer(this);
	connect(receiveTimer, &QTimer::timeout, this, &frmComTool::processReceivedData);
	receiveTimer->start(50); // 每50ms检查一次接收数据

	//ui->cboxSendInterval->addItems(AppData::Intervals);
	//ui->cboxData->addItems(AppData::Datas);

	//读取数据
	//timerRead = new QTimer(this);
	//timerRead->setInterval(100);
	//connect(timerRead, SIGNAL(timeout()), this, SLOT(readData()));

	//发送数据
	timerSend = new QTimer(this);
	connect(timerSend, SIGNAL(timeout()), this, SLOT(sendData()));
	//connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(sendData()));

	timerReLoad = new QTimer(this);
	connect(timerReLoad, SIGNAL(timeout()), this, SLOT(reLoad()));

	//保存数据
	timerSave = new QTimer(this);
	connect(timerSave, SIGNAL(timeout()), this, SLOT(saveData()));
	//connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(saveData()));

	//ui->tabWidget->setCurrentIndex(0);
	changeEnable(false);
	//ui->stackedWidget->setCurrentIndex(1);

	//ui->pushButton_SetTrigger_2->setEnabled(false);
	//ui->txtMain->setVisible(false);
	//ui->widgetRight->setVisible(false);
	//ui->widget_2->setVisible(false);
	//ui->widget->setVisible(false);
	//ui->frameTop->setVisible(false);

}

// 枚举可用串口的函数
QStringList frmComTool::enumerateSerialPorts()
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

void frmComTool::processReceivedData()
{
	if (!m_receiveQueue)
		return;

	while (!m_receiveQueue->isReceiveQueueEmpty()) {
		QByteArray data = m_receiveQueue->dequeueReceivedData();
		if (!data.isEmpty()) {
			// 处理接收到的数据

			append(1, QtHelperData::byteArrayToHexStr(data));


			uint8_t type = static_cast<uint8_t>(data.at(data.size() - 1));
			uint16_t resultData = 0;
			resultData = data[3] << 8 | data[4];
			if (type == 0x00)
			{
				//ui->lineEdit_Station->setText(QString::number(resultData));
			}
			else if (type == 0x01)
			{
				//ui->lineEdit_BT->setText(QString::number(resultData * 1200));
			}
			else if (type == 0x02)
			{
				//ui->lineEdit_Trigger->setText(QString::number(resultData));
			}
			else if (type == 0x10)
			{
				QString resultinfo = "";
				if (resultData == 0x00)
				{
					if (m_state == 2)
					{
						m_state = 3;
					}
					resultinfo = "待机";
				}
				else if (resultData == 0x01)
				{
					resultinfo = "触发";
				}
				else if (resultData == 0x02)
				{
					m_state = 2;
					resultinfo = "触发完成";
				}
				//ui->lineEdit_State->setText(resultinfo);
				//ui->labelState->setStyleSheet("color:rgb(220,220,222);");
				//ui->labelState->setText(resultinfo);
			}
			else if (type == 0x11)
			{
				m_state = 4;
				QString resultinfo = "";
				if (resultData == 0x00)
				{
					//ui->labelState 设置成红色
				//	ui->labelState->setStyleSheet("color:rgb(220,12,12);");
					resultinfo = "零值";
				}
				else if (resultData == 0x01)
				{
					//ui->labelState->setStyleSheet("color:rgb(12,220,12);");
					resultinfo = "正常";
				}
				else if (resultData == 0x02)
				{
					//ui->labelState->setStyleSheet("color:rgb(220,220,12);");
					resultinfo = "未知";
				}
				//	ui->lineEdit_Result->setText(resultinfo);
					//ui->labelState->setText(resultinfo);
			}
			else if (type == 0x12)
			{
				//ui->progressBar->setValue(resultData);
				//ui->labelPower->setText(QString("电量%1%").arg(QString::number(resultData)));
				//ui->battery->setValue((int)resultData);
			}

		}
	}
}

void frmComTool::openSerialPort()
{
	if (m_serialWorker && m_serialWorker->isRunning()) {
		return;
	}

	QString portName = ui->cboxPortName->currentText();
	// 获取其他串口参数...
	BaudRateType baudRate = (BaudRateType)ui->cboxBaudRate->currentText().toInt();
	DataBitsType dataBits = (DataBitsType)ui->cboxDataBit->currentText().toInt();
	ParityType parity = (ParityType)ui->cboxParity->currentIndex();
	StopBitsType stopBits = (StopBitsType)ui->cboxStopBit->currentIndex();

	m_serialWorker = new SerialWorker(m_sendQueue, m_receiveQueue);
	m_serialWorker->setPortSettings(portName, baudRate, dataBits, parity, stopBits, FLOW_OFF);

	connect(m_serialWorker, &SerialWorker::errorOccurred,
		this, &frmComTool::onSerialError);
	connect(m_serialWorker, &SerialWorker::portOpened,
		this, &frmComTool::onSerialPortOpened);
	connect(m_serialWorker, &SerialWorker::portClosed,
		this, &frmComTool::onSerialPortClosed);

	m_serialWorker->start();
}

void frmComTool::closeSerialPort()
{
	if (m_serialWorker) {
		m_serialWorker->stop();
		m_serialWorker->wait();
		m_serialWorker->deleteLater();
		m_serialWorker = nullptr;
	}
}

void frmComTool::onSerialPortOpened()
{
	changeEnable(true);
	append(0, QString("串口已打开"));
	ui->btnOpen->setText("关闭串口");
	//ui->labelComState->setText("已连接");
	//ui->pushButton_SetTrigger_2->setEnabled(true);
	sendData(Modbus::Modbus_Read_Power());
}

void frmComTool::onSerialPortClosed()
{
	changeEnable(false);
	append(0, QString("串口已关闭"));
	//ui->labelComState->setText("未连接");
	//ui->pushButton_SetTrigger_2->setEnabled(false);
	ui->btnOpen->setText("打开串口");
}

void frmComTool::onSerialError(const QString& error)
{
	append(6, error);
}

void frmComTool::initConfig()
{
	QStringList comList = enumerateSerialPorts();

	ui->cboxPortName->addItems(comList);
	ui->cboxPortName->setCurrentIndex(ui->cboxPortName->findText(AppConfig::PortName));
	connect(ui->cboxPortName, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	QStringList baudList;
	baudList << "50" << "75" << "100" << "134" << "150" << "200" << "300" << "600" << "1200"
		<< "1800" << "2400" << "4800" << "9600" << "14400" << "19200" << "38400"
		<< "56000" << "57600" << "76800" << "115200" << "128000" << "256000";

	ui->cboxBaudRate->addItems(baudList);
	ui->cboxBaudRate->setCurrentIndex(ui->cboxBaudRate->findText(QString::number(AppConfig::BaudRate)));
	connect(ui->cboxBaudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	QStringList dataBitsList;
	dataBitsList << "5" << "6" << "7" << "8";

	ui->cboxDataBit->addItems(dataBitsList);
	ui->cboxDataBit->setCurrentIndex(ui->cboxDataBit->findText(QString::number(AppConfig::DataBit)));
	connect(ui->cboxDataBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	QStringList parityList;
	parityList << "无" << "奇" << "偶";
#ifdef Q_OS_WIN
	parityList << "标志";
#endif
	parityList << "空格";

	ui->cboxParity->addItems(parityList);
	ui->cboxParity->setCurrentIndex(ui->cboxParity->findText(AppConfig::Parity));
	connect(ui->cboxParity, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	QStringList stopBitsList;
	stopBitsList << "1";
#ifdef Q_OS_WIN
	stopBitsList << "1.5";
#endif
	stopBitsList << "2";

	ui->cboxStopBit->addItems(stopBitsList);
	ui->cboxStopBit->setCurrentIndex(ui->cboxStopBit->findText(QString::number(AppConfig::StopBit)));
	connect(ui->cboxStopBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	//ui->ckHexSend->setChecked(AppConfig::HexSend);
	//connect(ui->ckHexSend, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	//ui->ckHexReceive->setChecked(AppConfig::HexReceive);
	//connect(ui->ckHexReceive, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	//ui->ckDebug->setChecked(AppConfig::Debug);
	//connect(ui->ckDebug, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	//ui->ckAutoClear->setChecked(AppConfig::AutoClear);
	//connect(ui->ckAutoClear, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	//ui->ckAutoSend->setChecked(AppConfig::AutoSend);
	//connect(ui->ckAutoSend, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	//ui->ckAutoSave->setChecked(AppConfig::AutoSave);
	//connect(ui->ckAutoSave, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

	QStringList sendInterval;
	QStringList saveInterval;
	sendInterval << "100" << "300" << "500";

	for (int i = 1000; i <= 10000; i = i + 1000) {
		sendInterval << QString::number(i);
		saveInterval << QString::number(i);
	}

	//ui->cboxSendInterval->addItems(sendInterval);
	//ui->cboxSaveInterval->addItems(saveInterval);

	//ui->cboxSendInterval->setCurrentIndex(ui->cboxSendInterval->findText(QString::number(AppConfig::SendInterval)));
	//connect(ui->cboxSendInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
	//ui->cboxSaveInterval->setCurrentIndex(ui->cboxSaveInterval->findText(QString::number(AppConfig::SaveInterval)));
	//connect(ui->cboxSaveInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	timerSend->setInterval(AppConfig::SendInterval);
	timerSave->setInterval(AppConfig::SaveInterval);

	if (AppConfig::AutoSend) {
		timerSend->start();
	}

	if (AppConfig::AutoSave) {
		timerSave->start();
	}

	//串口转网络部分
	//ui->cboxMode->setCurrentIndex(ui->cboxMode->findText(AppConfig::Mode));
	//connect(ui->cboxMode, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	//ui->txtServerIP->setText(AppConfig::ServerIP);
	//connect(ui->txtServerIP, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

	//ui->txtServerPort->setText(QString::number(AppConfig::ServerPort));
	//connect(ui->txtServerPort, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

	//ui->txtListenPort->setText(QString::number(AppConfig::ListenPort));
	//connect(ui->txtListenPort, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

	//QStringList values;
	//values << "0" << "10" << "50";

	//for (int i = 100; i < 1000; i = i + 100) {
	//    values << QString("%1").arg(i);
	//}

	//ui->cboxSleepTime->addItems(values);

	//ui->cboxSleepTime->setCurrentIndex(ui->cboxSleepTime->findText(QString::number(AppConfig::SleepTime)));
	//connect(ui->cboxSleepTime, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

	//ui->ckAutoConnect->setChecked(AppConfig::AutoConnect);
	//connect(ui->ckAutoConnect, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));
}


void frmComTool::on_cboxPortName_clicked(int nindex)
{
	// 保存当前选择的串口
	QString currentPort = ui->cboxPortName->currentText();

	// 清空现有项
	ui->cboxPortName->clear();

	// 获取可用串口列表
	QStringList portNames = enumerateSerialPorts();

	// 添加到下拉框
	ui->cboxPortName->addItems(portNames);

	// 恢复之前的选择（如果仍然存在）
	int index = ui->cboxPortName->findText(currentPort);
	if (index != -1) {
		ui->cboxPortName->setCurrentIndex(index);
	}
}

void frmComTool::saveConfig()
{
	AppConfig::PortName = ui->cboxPortName->currentText();
	AppConfig::BaudRate = ui->cboxBaudRate->currentText().toInt();
	AppConfig::DataBit = ui->cboxDataBit->currentText().toInt();
	AppConfig::Parity = ui->cboxParity->currentText();
	AppConfig::StopBit = ui->cboxStopBit->currentText().toDouble();

	//AppConfig::HexSend = ui->ckHexSend->isChecked();
	//AppConfig::HexReceive = ui->ckHexReceive->isChecked();
	//AppConfig::Debug = ui->ckDebug->isChecked();
	//AppConfig::AutoClear = ui->ckAutoClear->isChecked();

	//AppConfig::AutoSend = ui->ckAutoSend->isChecked();
	//AppConfig::AutoSave = ui->ckAutoSave->isChecked();

	//int sendInterval = ui->cboxSendInterval->currentText().toInt();
	//if (sendInterval != AppConfig::SendInterval) {
	//	AppConfig::SendInterval = sendInterval;
	//	timerSend->setInterval(AppConfig::SendInterval);
	//}

	//int saveInterval = ui->cboxSaveInterval->currentText().toInt();
	//if (saveInterval != AppConfig::SaveInterval) {
	//	AppConfig::SaveInterval = saveInterval;
	//	timerSave->setInterval(AppConfig::SaveInterval);
	//}

	//AppConfig::Mode = ui->cboxMode->currentText();
	//AppConfig::ServerIP = ui->txtServerIP->text().trimmed();
	//AppConfig::ServerPort = ui->txtServerPort->text().toInt();
	//AppConfig::ListenPort = ui->txtListenPort->text().toInt();
	//AppConfig::SleepTime = ui->cboxSleepTime->currentText().toInt();
	//AppConfig::AutoConnect = ui->ckAutoConnect->isChecked();

	AppConfig::writeConfig();
}

void frmComTool::changeEnable(bool b)
{
	ui->cboxBaudRate->setEnabled(!b);
	ui->cboxDataBit->setEnabled(!b);
	ui->cboxParity->setEnabled(!b);
	ui->cboxPortName->setEnabled(!b);
	ui->cboxStopBit->setEnabled(!b);
	//ui->btnSend->setEnabled(b);
	//ui->ckAutoSend->setEnabled(b);
	//ui->ckAutoSave->setEnabled(b);
	//ui->progressBar->setValue(0);
}

void frmComTool::append(int type, const QString& data, bool clear)
{
	static int currentCount = 0;
	static int maxCount = 100;

	if (clear) {
		//ui->txtMain->clear();
		currentCount = 0;
		return;
	}

	if (currentCount >= maxCount) {
		//ui->txtMain->clear();
		currentCount = 0;
	}

	//过滤回车换行符
	QString strData = data;
	strData = strData.replace("\r", "");
	strData = strData.replace("\n", "");

	//不同类型不同颜色显示
	QString strType;
	if (type == 0) {
		strType = "串口发送 >>";
		//ui->txtMain->setTextColor(QColor("dodgerblue"));
	}
	else if (type == 1) {
		strType = "串口接收 <<";
		//ui->txtMain->setTextColor(QColor("red"));
	}
	else if (type == 2) {
		strType = "处理延时 >>";
		//ui->txtMain->setTextColor(QColor("gray"));
	}
	else if (type == 3) {
		strType = "正在校验 >>";
		//ui->txtMain->setTextColor(QColor("green"));
	}
	else if (type == 4) {
		strType = "网络发送 >>";
		//ui->txtMain->setTextColor(QColor(24, 189, 155));
	}
	else if (type == 5) {
		strType = "网络接收 <<";
		//ui->txtMain->setTextColor(QColor(255, 107, 107));
	}
	else if (type == 6) {
		strType = "提示信息 >>";
		//ui->txtMain->setTextColor(QColor(100, 184, 255));
	}

	strData = QString("时间[%1] %2 %3").arg(TIMEMS).arg(strType).arg(strData);
	//ui->txtMain->append(strData);
	currentCount++;
}

void frmComTool::readData()
{
	//if (com->bytesAvailable() <= 0) {
	//	return;
	//}

	//QtHelper::sleep(sleepTime);
	//QByteArray data = com->readAll();
	//int dataLen = data.length();
	//if (dataLen <= 0) {
	//	return;
	//}

	//if (isShow) {
	//	QString buffer;
	//	if (ui->ckHexReceive->isChecked()) {
	//		buffer = QtHelperData::byteArrayToHexStr(data);
	//	}
	//	else {
	//		//buffer = QtHelperData::byteArrayToAsciiStr(data);
	//		buffer = QString::fromLocal8Bit(data);
	//	}

	//	//启用调试则模拟调试数据
	//	if (ui->ckDebug->isChecked()) {
	//		int count = AppData::Keys.count();
	//		for (int i = 0; i < count; i++) {
	//			if (buffer.startsWith(AppData::Keys.at(i))) {
	//				sendData(AppData::Values.at(i));
	//				break;
	//			}
	//		}
	//	}

	//	append(1, buffer);
	//	receiveCount = receiveCount + data.size();
	//	ui->btnReceiveCount->setText(QString("接收 : %1 字节").arg(receiveCount));

	//	//启用网络转发则调用网络发送数据
	//	if (tcpOk) {
	//		socket->write(data);
	//		append(4, QString(buffer));
	//	}
	//}
}

void frmComTool::readData(uint8_t type)
{
	int count = 0;
	do
	{
		count++;
		//if (com->bytesAvailable() <= 0) {
		//	QtHelper::sleep(1);
		//	continue;
		//}
		QtHelper::sleep(sleepTime);
		QByteArray data;// = com->readAll();
		int dataLen = data.length();
		if (dataLen <= 0) {
			return;
		}

		if (isShow) {
			QString buffer;
			//if (ui->ckHexReceive->isChecked()) {
				//buffer = QtHelperData::byteArrayToHexStr(data);
			//}
			//else {
				//buffer = QtHelperData::byteArrayToAsciiStr(data);
			//	buffer = QString::fromLocal8Bit(data);
			//}

			//启用调试则模拟调试数据
			//if (ui->ckDebug->isChecked()) {
			//	int count = AppData::Keys.count();
			//	for (int i = 0; i < count; i++) {
				//	if (buffer.startsWith(AppData::Keys.at(i))) {
				//		sendData(AppData::Values.at(i));
				//		break;
				//	}
			//	}
			//}

			//append(1, buffer);
			receiveCount = receiveCount + data.size();
			//ui->btnReceiveCount->setText(QString("接收 : %1 字节").arg(receiveCount));
		}
	} while (count < 100);
}

void  frmComTool::on_pushButton_clicked()
{
	QString strData = ui->textEdit->toPlainText();
	// 使用您提供的十六进制字符串
	//std::string userHexString = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";

	CommunicationProtocol protocol;
	if (protocol.buildFromHexString(strData.toStdString()))
	{
		qDebug() << "成功解析用户提供的十六进制字符串";
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

		ui->plainTextEdit->appendPlainText(QString("报文类型：%1;编码：%2").arg(getPacketTypeName(protocol.packetTypeCode)).arg(QString("0x%1").arg(protocol.packetTypeCode, 0, 16).toUpper()));
		std::string standardXml = protocol.getServiceDataString();
		// 如果plainTextEdit是QPlainTextEdit类型，可以直接追加
		ui->plainTextEdit->appendPlainText(formatXmlString(standardXml));
		ui->plainTextEdit->setFont(QFont("Consolas", 10)); // 等宽字体，显示更美观

		// 验证 CRC
		if (protocol.verifyCRC32())
		{
			qDebug() << "CRC32 校验通过";
		}
		else
		{
			qDebug() << "CRC32 校验失败（可能是示例字符串的 CRC 值不匹配）";
		}
	}
	else
	{
		qDebug() << "解析失败";
	}

	// 创建 ImageData 对象并解析
	ImageData imageData;

	if (imageData.parseFromBytes(protocol.detectionFileData))
	{
		qDebug() << "=== 解析成功 ===";

		// 1. 基本信息
		qDebug() << "文件长度：" << imageData.fileLength << " 字节";
		qDebug() << "规范版本号：" << imageData.getVersionString();
		qDebug() << "生成时间：" << imageData.getCreateTimeString();
		qDebug() << "站点名称：" << imageData.getStationNameString();
	}
	else
	{
		qDebug() << "解析图像数据失败！";
	}

	// 创建红外图谱数据对象
	InfraredSpectrumData spectrumData;

	// 解析 detectionFileData
	if (spectrumData.parseFromBytes(protocol.detectionFileData))
	{
		// 解析成功，访问各个字段

		// 1. 基本信息
		qDebug() << "数据类型编码：" << (int)spectrumData.dataTypeCode;
		qDebug() << "图谱数据长度：" << spectrumData.dataLength;
		qDebug() << "生成时间：" << spectrumData.getCreateTimeString();
		qDebug() << "图谱性质：" << spectrumData.getSpectrumPropertyDescription();

		// 2. 设备信息
		qDebug() << "设备名称：" << spectrumData.getDeviceNameString();
		qDebug() << "设备编码：" << spectrumData.getDeviceCodeString();
		qDebug() << "测点名称：" << spectrumData.getMeasurementPointNameString();
		qDebug() << "测点编码：" << spectrumData.getMeasurementPointCodeString();

		// 3. 检测参数
		qDebug() << "检测通道标志：" << spectrumData.detectionChannelFlag;
		qDebug() << "存储器数据类型：" << (int)spectrumData.storageDataType;
		qDebug() << "温度单位：" << (int)spectrumData.temperatureUnit;

		// 4. 图像参数
		qDebug() << "温度矩阵宽度：" << spectrumData.temperatureMatrixWidth;
		qDebug() << "温度矩阵高度：" << spectrumData.temperatureMatrixHeight;

		// 5. 照片数据长度
		qDebug() << "可见光照片长度：" << spectrumData.visibleLightDataLength;
		qDebug() << "红外照片长度：" << spectrumData.infraredPhotoDataLength;

		// 6. 环境参数
		qDebug() << "辐射率：" << spectrumData.emissivity;
		qDebug() << "测试距离：" << spectrumData.testDistance << " m";
		qDebug() << "大气温度：" << spectrumData.atmosphericTemperature << " °C";
		qDebug() << "相对湿度：" << (int)spectrumData.relativeHumidity << " %";
		qDebug() << "反射温度：" << spectrumData.reflectedTemperature << " °C";

		// 7. 温宽参数
		qDebug() << "温宽上限：" << spectrumData.temperatureRangeUpper;
		qDebug() << "温宽下限：" << spectrumData.temperatureRangeLower;

		// 8. 访问照片数据
		if (!spectrumData.visibleLightPhotoData.empty())
		{
			qDebug() << "可见光照片数据大小：" << spectrumData.visibleLightPhotoData.size() << " 字节";
			// 可以将 visibleLightPhotoData 保存到文件或显示
		}

		if (!spectrumData.infraredPhotoData.empty())
		{
			qDebug() << "红外照片数据大小：" << spectrumData.infraredPhotoData.size() << " 字节";
			// 可以将 infraredPhotoData 保存到文件或显示
		}
	}
	else
	{
		qDebug() << "解析红外图谱数据失败!";
	}
}

void frmComTool::on_pushButton_3_clicked()
{
	std::string DataConfer = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";
	CommunicationProtocol protocol;
	if (protocol.buildFromHexString(DataConfer))
	{
		qDebug() << "✓ 获取标准XML数据成功";
		sendData(protocol.toBytes());
	}
}

void frmComTool::on_pushButton_4_clicked()
{
}

QString frmComTool::getPacketTypeName(uint32_t packetTypeCode)
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


QString frmComTool::formatXmlString(const std::string& str)
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

void frmComTool::sendData()
{
	//
	//
	if (m_state == 0)
	{
		sendData(Modbus::Modbus_Read_Power());
	}
	else if (m_state == 1 || m_state == 2)
	{
		sendData(Modbus::Modbus_Read_Detection_State());
	}
	else if (m_state == 3)
	{
		sendData(Modbus::Modbus_Read_Detection_Result());
	}
	else if (m_state == 4)
	{
		timerSend->setInterval(AppConfig::SendInterval);
		m_state = 0;
	}

}

void frmComTool::sendData(QString data)
{
	//if (com == 0 || !com->isOpen()) {
	//	return;
	//}

	////短信猫调试
	//if (data.startsWith("AT")) {
	//	data += "\r";
	//}

	//QByteArray buffer;
	//if (ui->ckHexSend->isChecked()) {
	//	buffer = QtHelperData::hexStrToByteArray(data);
	//}
	//else {
	//	buffer = QtHelperData::asciiStrToByteArray(data);
	//}

	//com->write(buffer);
	//append(0, data);
	//sendCount = sendCount + buffer.size();
	//ui->btnSendCount->setText(QString("发送 : %1 字节").arg(sendCount));
}

void frmComTool::sendData(std::vector<uint8_t> data)
{
	//if (com == 0 || !com->isOpen()) {
	//	return;
	//}

	//std::vector<uint8_t> 转成QTypeBuffer
	QByteArray buffer1;
	buffer1.append((char*)data.data(), data.size());
	////QByteArray buffer;
	if (m_sendQueue) {
		m_sendQueue->enqueueSendData(buffer1);
	}

	//将data 转成字符串
	QString buffer = QtHelperData::vectorToHexStr(data);

	append(0, buffer);
	sendCount = sendCount + data.size();
	//ui->btnSendCount->setText(QString("发送 : %1 字节").arg(sendCount));
}

void frmComTool::sendData(const QByteArray& data)
{
	if (m_sendQueue) {
		m_sendQueue->enqueueSendData(data);
	}
}

void frmComTool::saveData()
{
	//QString tempData = ui->txtMain->toPlainText();
	//if (tempData.isEmpty()) {
	//	return;
	//}

	QDateTime now = QDateTime::currentDateTime();
	QString name = now.toString("yyyy-MM-dd-HH-mm-ss");
	QString fileName = QString("%1/%2.txt").arg(QtHelper::appPath()).arg(name);

	QFile file(fileName);
	file.open(QFile::WriteOnly | QIODevice::Text);
	QTextStream out(&file);
	//out << tempData;
	file.close();

	on_btnClear_clicked();
}

void frmComTool::reLoad()
{
	if (reloadtimes > 0) {
		timerReLoad->start(1000);
		//ui->pushButton_SetTrigger_2->setText(QString("充电中..%1s").arg(reloadtimes));
		reloadtimes--;
		return;
	}
	reloadtimes = 10;
	timerReLoad->stop();
	//ui->pushButton_SetTrigger_2->setEnabled(true);
	//ui->pushButton_SetTrigger_2->setText("");
}

void frmComTool::on_btnOpen_clicked()
{
	if (ui->btnOpen->text() == "打开串口") {
		openSerialPort();

		//com = new QextSerialPort(ui->cboxPortName->currentText(), QextSerialPort::Polling);
		//comOk = com->open(QIODevice::ReadWrite);

		//if (comOk) {
		//	//清空缓冲区
		//	com->flush();
		//	//设置波特率
		//	com->setBaudRate((BaudRateType)ui->cboxBaudRate->currentText().toInt());
		//	//设置数据位
		//	com->setDataBits((DataBitsType)ui->cboxDataBit->currentText().toInt());
		//	//设置校验位
		//	com->setParity((ParityType)ui->cboxParity->currentIndex());
		//	//设置停止位
		//	com->setStopBits((StopBitsType)ui->cboxStopBit->currentIndex());
		//	com->setFlowControl(FLOW_OFF);
		//	com->setTimeout(10);

		//	changeEnable(true);
		//	ui->btnOpen->setText("关闭串口");
		//	//timerRead->start();
		//}
	}
	else {
		closeSerialPort();

		changeEnable(false);
		ui->btnOpen->setText("打开串口");
		//on_btnClear_clicked();
	}
}

void frmComTool::on_btnSendCount_clicked()
{
	sendCount = 0;
	//ui->btnSendCount->setText("发送 : 0 字节");
}

void frmComTool::on_btnReceiveCount_clicked()
{
	receiveCount = 0;
	//ui->btnReceiveCount->setText("接收 : 0 字节");
}
void frmComTool::on_pushButton_ReadStation_clicked()
{
	//ui->lineEdit_Station->setText("");
	sendData(Modbus::Modbus_Read_Station());
	//readData(0x00);
}
void frmComTool::on_pushButton_ReadBT_clicked()
{
	//ui->lineEdit_BT->setText("");
	sendData(Modbus::Modbus_Read_BT());
	//readData(0x01);
}
void frmComTool::on_pushButton_ReadTrigger_clicked()
{
	//ui->lineEdit_Trigger->setText("");
	sendData(Modbus::Modbus_Read_Trigger());
	//readData(0x02);
}
void frmComTool::on_pushButton_SetTrigger_clicked()
{
	sendData(Modbus::Modbus_Set_Trigger(0x55));
	//readData(0x02);
}
void frmComTool::on_pushButton_SetTrigger_2_clicked()
{

	//	ui->pushButton_SetTrigger_2->setEnabled(false);
		//ui->pushButton_SetTrigger_2->setText("充电中。。");
	timerReLoad->start(1000);
	m_state = 1;
	timerSend->setInterval(1000);
	sendData(Modbus::Modbus_Set_Trigger(0x55));
}
void frmComTool::on_pushButton_ReadState_clicked()
{
	//ui->lineEdit_State->setText("");
	sendData(Modbus::Modbus_Read_Detection_State());
	//readData(0x10);
}
void frmComTool::on_pushButton_ReadResult_clicked()
{
	//ui->lineEdit_Result->setText("");
	sendData(Modbus::Modbus_Read_Detection_Result());
	//readData(0x11);
}
void frmComTool::on_pushButton_ReadBattery_clicked()
{
	//ui->progressBar->setValue(0);
	sendData(Modbus::Modbus_Read_Power());
	//readData(0x12);
}

//
//void frmComTool::on_btnData_clicked()
//{
//    QString fileName = QString("%1/%2").arg(QtHelper::appPath()).arg("send.txt");
//    QFile file(fileName);
//    if (!file.exists()) {
//        return;
//    }
//
//    if (ui->btnData->text() == "管理数据") {
//        ui->txtMain->setReadOnly(false);
//        ui->txtMain->clear();
//        file.open(QFile::ReadOnly | QIODevice::Text);
//        QTextStream in(&file);
//        ui->txtMain->setText(in.readAll());
//        file.close();
//        ui->btnData->setText("保存数据");
//    } else {
//        ui->txtMain->setReadOnly(true);
//        file.open(QFile::WriteOnly | QIODevice::Text);
//        QTextStream out(&file);
//        out << ui->txtMain->toPlainText();
//        file.close();
//        ui->txtMain->clear();
//        ui->btnData->setText("管理数据");
//        AppData::readSendData();
//    }
//}

void frmComTool::on_btnClear_clicked()
{
	append(0, "", true);
}

//void frmComTool::on_btnStart_clicked()
//{
	//if (ui->btnStart->text() == "启动") {
	//    if (AppConfig::ServerIP == "" || AppConfig::ServerPort == 0) {
	//        append(6, "IP地址和远程端口不能为空");
	//        return;
	//    }

	//    socket->connectToHost(AppConfig::ServerIP, AppConfig::ServerPort);
	//    if (socket->waitForConnected(100)) {
	//        ui->btnStart->setText("停止");
	//        append(6, "连接服务器成功");
	//        tcpOk = true;
	//    }
	//} else {
	//    socket->disconnectFromHost();
	//    if (socket->state() == QAbstractSocket::UnconnectedState || socket->waitForDisconnected(100)) {
	//        ui->btnStart->setText("启动");
	//        append(6, "断开服务器成功");
	//        tcpOk = false;
	//    }
	//}
//}

void frmComTool::on_ckAutoSend_stateChanged(int arg1)
{
	if (arg1 == 0) {
		//	ui->cboxSendInterval->setEnabled(false);
		timerSend->stop();
	}
	else {
		//	ui->cboxSendInterval->setEnabled(true);
		timerSend->start();
	}
}

void frmComTool::on_ckAutoSave_stateChanged(int arg1)
{
	if (arg1 == 0) {
		//ui->cboxSaveInterval->setEnabled(false);
		timerSave->stop();
	}
	else {
		//ui->cboxSaveInterval->setEnabled(true);
		timerSave->start();
	}
}
