#include "frmcomtool.h"
#include "ui_frmcomtool.h"
#include "qthelper.h"
#include "qthelperdata.h"
#include <CommunicationProtocol.h>
#include <InfraredSpectrumData.h>
#include <ImageData.h>
#include <QByteArray>

frmComTool::frmComTool(QWidget* parent) : QWidget(parent), ui(new Ui::frmComTool)
{
	ui->setupUi(this);
	this->initForm();
	this->initConfig();
	openSerialPort();
	initTreeWidget();
	QtHelper::setFormInCenter(this);
}

void frmComTool::initTreeWidget()
{
	imageitem = nullptr;
	infraredspectrumitem = nullptr;
	// 连接信号槽
	bool isConnected = connect(ui->treeWidget, &QTreeWidget::itemClicked,
		this, &frmComTool::onTreeItemClicked);
	qDebug() << "Connection successful:" << isConnected;

	// 自动调整列宽
	ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	ui->treeWidget_2->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->treeWidget_2->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	//间隔颜色显示
	ui->treeWidget->setAlternatingRowColors(true);
    ui->treeWidget_2->setAlternatingRowColors(true);
	// 设置行高
    ui->treeWidget_2->setStyleSheet("QTreeWidget::item { height: 25px; }");

	addTreeConmunicationItem();

	//QTreeWidgetItem*  = findItemByName("检测数据文件");
	if (imageitem)
		addTreeItemImageData(imageitem);

}

void frmComTool::parseXmlToTree(const QString& xmlData)
{
	QXmlStreamReader reader(xmlData);
	QTreeWidgetItem* currentParent = nullptr;

	// 逐行解析XML
	while (!reader.atEnd()) {
		if (reader.readNextStartElement()) {
			// 解析主任务
			if (reader.name() == "main_task") {
				QString name = reader.attributes().value("name").toString();
				currentParent = new QTreeWidgetItem(ui->treeWidget);
				currentParent->setText(0, name);
				currentParent->setExpanded(true);
			}
			// 解析子任务
			else if (reader.name() == "sub_task") {
				QString name = reader.attributes().value("name").toString();
				addTreeNode(currentParent, name);
				currentParent = currentParent->child(currentParent->childCount() - 1);
			}
			// 解析间隔(clearance)
			else if (reader.name() == "clearance") {
				QString name = reader.attributes().value("name").toString();
				addTreeNode(currentParent, name);
				currentParent = currentParent->child(currentParent->childCount() - 1);
			}
			// 解析测试点
			else if (reader.name() == "test_point") {
				QString name = reader.attributes().value("name").toString();
				addTreeNode(currentParent, name);
			}
		}
	}

	// 异常处理
	if (reader.hasError()) {
		qDebug() << "XML解析错误：" << reader.errorString();
	}
}

void frmComTool::addTreeNode(QTreeWidgetItem* parentItem, const QString& nodeName)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
	item->setText(0, nodeName);
	// 自动展开节点
	item->setExpanded(true);
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
		if (!data.isEmpty())
		{
			CommunicationProtocol protocol;
			// 从QByteArray 到std::vector<uint8_t>
			std::vector<uint8_t> dataVector(data.begin(), data.end());

			if (protocol.buildFromBytes(dataVector))
			{
				if (protocol.packetTypeCode == 0x00000001)
				{
					parseXmlToTree(formatXmlString(protocol.getServiceDataString()));
				}

			}
			else
			{
				qDebug() << "CommunicationProtocol::buildFromBytes() failed";
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

int frmComTool::getTreeItemLevel(QTreeWidgetItem* item)
{
	int level = 0;
	// 循环往上找父节点，有一个父节点就层级+1
	while (item->parent() != nullptr) {
		level++;
		item = item->parent();
	}
	return level;
}

void frmComTool::addTreeConmunicationItem()
{
	// 1 报文头
	QTreeWidgetItem* item1 = new QTreeWidgetItem(ui->treeWidget_2);
	item1->setText(0, "报文头");
	item1->setText(1, "uint32_t (4字节)");

	// 2 版本号
	QTreeWidgetItem* item2 = new QTreeWidgetItem(ui->treeWidget_2);
	item2->setText(0, "版本号");
	item2->setText(1, "uint8_t (1字节)");

	// 3 序号
	QTreeWidgetItem* item3 = new QTreeWidgetItem(ui->treeWidget_2);
	item3->setText(0, "序号");
	item3->setText(1, "uint16_t (2字节)");

	// 4 请求标志
	QTreeWidgetItem* item4 = new QTreeWidgetItem(ui->treeWidget_2);
	item4->setText(0, "请求标志");
	item4->setText(1, "uint8_t (1字节)");

	// 5 数据包总长度
	QTreeWidgetItem* item5 = new QTreeWidgetItem(ui->treeWidget_2);
	item5->setText(0, "数据包总长度");
	item5->setText(1, "uint64_t (8字节)");

	// 6 报文类型编码
	QTreeWidgetItem* item6 = new QTreeWidgetItem(ui->treeWidget_2);
	item6->setText(0, "报文类型编码");
	item6->setText(1, "uint32_t (4字节)");

	// 7 压缩标志
	QTreeWidgetItem* item7 = new QTreeWidgetItem(ui->treeWidget_2);
	item7->setText(0, "压缩标志");
	item7->setText(1, "uint8_t (1字节)");

	// 8 加密标志
	QTreeWidgetItem* item8 = new QTreeWidgetItem(ui->treeWidget_2);
	item8->setText(0, "加密标志");
	item8->setText(1, "uint8_t (1字节)");

	// 9 仪器厂商
	QTreeWidgetItem* item9 = new QTreeWidgetItem(ui->treeWidget_2);
	item9->setText(0, "仪器厂商");
	item9->setText(1, "uint8_t (1字节)");

	// 10 备用
	QTreeWidgetItem* item10 = new QTreeWidgetItem(ui->treeWidget_2);
	item10->setText(0, "备用");
	item10->setText(1, "uint8_t[15] (15字节)");

	// 11 业务数据格式
	QTreeWidgetItem* item11 = new QTreeWidgetItem(ui->treeWidget_2);
	item11->setText(0, "业务数据格式");
	item11->setText(1, "uint8_t (1字节)");

	// 12 业务数据长度
	QTreeWidgetItem* item12 = new QTreeWidgetItem(ui->treeWidget_2);
	item12->setText(0, "业务数据长度");
	item12->setText(1, "uint64_t (8字节)");

	// 13 业务数据
	QTreeWidgetItem* item13 = new QTreeWidgetItem(ui->treeWidget_2);
	item13->setText(0, "业务数据");
	item13->setText(1, "uint8_t 数组 (动态)");

	// 14 检测数据文件长度
	QTreeWidgetItem* item14 = new QTreeWidgetItem(ui->treeWidget_2);
	item14->setText(0, "检测数据文件长度");
	item14->setText(1, "uint64_t (8字节)");

	// 15 检测数据文件
	QTreeWidgetItem* item15 = new QTreeWidgetItem(ui->treeWidget_2);
	item15->setText(0, "检测数据文件");
	item15->setText(1, "uint8_t 数组 (动态)");
	imageitem = item15;

	// 16 校验字节
	QTreeWidgetItem* item16 = new QTreeWidgetItem(ui->treeWidget_2);
	item16->setText(0, "校验字节(CRC32)");
	item16->setText(1, "uint32_t (4字节)");

	// 17 报文尾
	QTreeWidgetItem* item17 = new QTreeWidgetItem(ui->treeWidget_2);
	item17->setText(0, "报文尾");
	item17->setText(1, "uint8_t (1字节)");
}

void frmComTool::addTreeItemImageData(QTreeWidgetItem* parentItem)
{
	// 1 文件长度 L
	addChildItem(parentItem, "文件长度 L", "int32 (4字节)");

	// 2 规范版本号
	addChildItem(parentItem, "规范版本号", "uint8[4] (4字节)");

	// 3 文件生成时间
	addChildItem(parentItem, "文件生成时间", "int64 (8字节)");

	// 4 站点名称
	addChildItem(parentItem, "站点名称", "char[118] (118字节)");

	// 5 站点编码
	addChildItem(parentItem, "站点编码", "char[42] (42字节)");

	// 6 天气
	addChildItem(parentItem, "天气", "uint8 (1字节)");

	// 7 温度
	addChildItem(parentItem, "温度", "float (4字节)");

	// 8 湿度
	addChildItem(parentItem, "湿度", "uint8 (1字节)");

	// 9 仪器厂家
	addChildItem(parentItem, "仪器厂家", "char[32] (32字节)");

	// 10 仪器型号
	addChildItem(parentItem, "仪器型号", "char[32] (32字节)");

	// 11 仪器版本号
	addChildItem(parentItem, "仪器版本号", "uint8[4] (4字节)");

	// 12 仪器序列号
	addChildItem(parentItem, "仪器序列号", "char[32] (32字节)");

	// 13 系统频率
	addChildItem(parentItem, "系统频率", "float (4字节)");

	// 14 图谱数量 N
	addChildItem(parentItem, "图谱数量 N", "int16 (2字节)");

	// 15 经度
	addChildItem(parentItem, "经度", "double (8字节)");

	// 16 纬度
	addChildItem(parentItem, "纬度", "double (8字节)");

	// 17 海拔
	addChildItem(parentItem, "海拔", "int32 (4字节)");

	// 18 预留
	addChildItem(parentItem, "预留", "byte[204] (204字节)");

	// 19 图谱数据
	addChildItem(parentItem, "图谱数据", "动态长度");

	infraredspectrumitem = findItemByName("图谱数据");

	// 20 文件尾部预留
	addChildItem(parentItem, "文件尾部预留", "byte[32] (32字节)");

	// 21 CRC32
	addChildItem(parentItem, "CRC32", "int32 (4字节)");
}

void frmComTool::addTreeItemInfraredSpectrumData(QTreeWidgetItem* parentItem)
{

}


// 工具函数：添加二级子节点
void frmComTool::addChildItem(QTreeWidgetItem* parent, const QString& name, const QString& type)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(parent);
	item->setText(0, name);
	item->setText(1, type);
	item->setExpanded(true);
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
		QString xmlData = formatXmlString(standardXml);
		ui->plainTextEdit->appendPlainText(xmlData);
		parseXmlToTree(xmlData);
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
	// 将protocol.detectionFileData 保存成zip文件
	//qDebug() << "保存检测文件...";
	//qt_gzip_save_file("detection_file.zip", protocol.detectionFileData);
	//qDebug() << "检测文件保存成功！";

	//// 解压
	//if (protocol.compressionFlag == 1)
	//{

	//	qDebug() << "开始解压...";
	//	std::vector<uint8_t> uncompressedData = qt_gzip_compress(protocol.detectionFileData);
	//	if (!uncompressedData.empty())
	//	{
	//		qDebug() << "解压成功！";
	//		protocol.setDetectionFile(uncompressedData);
	//	}
	//	else
	//	{
	//		qDebug() << "解压失败！";
	//	}
	//}

	// 从本地文件中读取二进制数据

}


// 功能：将多个文件压缩到 内存ZIP 数据流
//bool frmComTool::zip_mem_compress(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files,std::vector<uint8_t>& zip_data) {
	//// 1. 创建内存ZIP
	//zlib_filefunc_def zff;
	//fill_fopen_filefunc(&zff);

	//zipFile zf = zipOpen2(nullptr, APPEND_STATUS_CREATE, nullptr, &zff);
	//if (!zf) return false;

	//// 2. 逐个添加文件
	//for (const auto& file : files) {
	//	const std::string& filename = file.first;
	//	const std::vector<uint8_t>& data = file.second;

	//	zipOpenNewFileInZip(
	//		zf,
	//		filename.c_str(),
	//		nullptr,
	//		nullptr, 0,
	//		nullptr, 0,
	//		nullptr,
	//		Z_DEFLATED,
	//		Z_DEFAULT_COMPRESSION
	//	);

	//	zipWriteInFileInZip(zf, data.data(), data.size());
	//	zipCloseFileInZip(zf);
	//}

	//// 3. 关闭并获取内存ZIP
	//zipClose(zf, nullptr);

	//// 这里省略从内存获取ZIP数据的代码（需要ioapi_mem配合）
	//// 完整代码我可以一次性给你全套
	//return true;
//}

// Qt 原生解 GZIP（适配电力规约报文）
//QByteArray frmComTool::qt_gzip_decompress(const QByteArray& gzipData)
//{
	//if (gzipData.isEmpty())
	//	return QByteArray();

	//z_stream strm;
	//memset(&strm, 0, sizeof(z_stream));

	//// 重点：告诉zlib这是标准gzip格式
	//if (inflateInit2(&strm, MAX_WBITS | 32) != Z_OK) {
	//	qDebug() << "zlib初始化失败";
	//	return QByteArray();
	//}

	//strm.next_in = (Bytef*)gzipData.constData();
	//strm.avail_in = (uInt)gzipData.size();

	//QByteArray result;
	//const int BUF_SIZE = 4096;
	//unsigned char buffer[BUF_SIZE];

	//do {
	//	strm.avail_out = BUF_SIZE;
	//	strm.next_out = buffer;

	//	int ret = inflate(&strm, Z_NO_FLUSH);
	//	if (ret < 0 && ret != Z_STREAM_END) {
	//		qDebug() << "解压错误:" << ret;
	//		inflateEnd(&strm);
	//		return QByteArray();
	//	}

	//	int have = BUF_SIZE - strm.avail_out;
	//	result.append((char*)buffer, have);

	//} while (strm.avail_out == 0);

	//inflateEnd(&strm);
	//return result;
//}

//std::vector<uint8_t> frmComTool::qt_gzip_compress(std::vector<uint8_t> gzipData)
//{
	// 将std::vector 转换为 QByteArray
  //  QByteArray zlibData = QByteArray::fromRawData(reinterpret_cast<const char*>(gzipData.data()), gzipData.size());
	//QByteArray result = qt_gzip_decompress(zlibData);
  //  return std::vector<uint8_t>(result.begin(), result.end());
//}

std::vector<uint8_t> frmComTool::qt_gzip_load_file(std::string filepath)
{
	QFile file(filepath.c_str());
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "无法打开文件" << filepath;
		return std::vector<uint8_t>();
	}
	QByteArray fileData = file.readAll();
	return std::vector<uint8_t>(fileData.begin(), fileData.end());

}

bool frmComTool::qt_gzip_save_file(const std::string& fileName, const std::vector<uint8_t>& data)
{
	QFile file(fileName.c_str());
	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "无法打开文件" << fileName;
		return false;
	}
	if (file.write(QByteArray::fromRawData(reinterpret_cast<const char*>(data.data()), data.size())) == data.size())
	{
		qDebug() << "文件保存成功";
		file.close();
		return true;
	}
	qDebug() << "文件保存失败";
	return false;
}

bool frmComTool::zip_mem_compress(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files, std::vector<uint8_t>& zip_data)
{
	return false;
}

void frmComTool::on_pushButton_2_clicked()
{
	std::string xmlData = ui->textEdit->toPlainText().toStdString();

	// 将十六进制字符串转换为字节数组
	std::vector<uint8_t> data;
	data.reserve(xmlData.length() / 2);

	for (size_t i = 0; i < xmlData.length(); i += 2)
	{
		std::string byteString = xmlData.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
		data.push_back(byte);
	}
	sendData(data);

	//将数据保存成dat文件
	qt_gzip_save_file("D:/test.dat", data);
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

void frmComTool::on_pushButton_5_clicked()
{
	std::string filePath = "E:\\GitHub\\Multi-protocol-Data-Terminal\\QtZipWriterAndReader\\unzip_folde1\\1000kV主变测试1_1000kV主变测试_point1_主变1_750kV母线1  本体.dat";
	std::vector<uint8_t> fileData = qt_gzip_load_file(filePath);

	// 创建 ImageData 对象并解析
	ImageData imageData;

	if (imageData.parseFromBytes(fileData))
	{
		qDebug() << "=== 解析成功 ===";

		// 1. 基本信息
		qDebug() << "文件长度：" << imageData.fileLength << " 字节";
		qDebug() << "规范版本号：" << imageData.getVersionString();
		qDebug() << "生成时间：" << imageData.getCreateTimeString();
		qDebug() << "站点名称：" << imageData.getStationNameString();
		qDebug() << "站点代码：" << imageData.getStationCodeString();
		qDebug() << "仪器型号" << imageData.getInstrumentModelString();
		qDebug() << "天气" << imageData.getWeatherDescription();

		for (int i = 0; i < imageData.spectrumCount; i++)
		{
			InfraredSpectrumData infraredSpectrumData;
			if (infraredSpectrumData.parseFromBytes(imageData.spectrumData[i]))
			{
				qDebug() << "数据长度：" << infraredSpectrumData.dataLength;
				qDebug() << "数据类型：" << infraredSpectrumData.dataTypeCode;
				qDebug() << "生成时间：" << infraredSpectrumData.getCreateTimeString();
			}
		}
	}
	else
	{
		qDebug() << "解析图像数据失败！";
	}
}

QTreeWidgetItem* frmComTool::findItemByName(const QString& name)
{
	// 遍历所有一级节点
	for (int i = 0; i < ui->treeWidget_2->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = ui->treeWidget_2->topLevelItem(i);
		if (item->text(0) == name) {
			return item; // 找到返回
		}
	}
	return nullptr; // 没找到
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

void frmComTool::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	if (!item) return;

	QString text = item->text(column);
	qDebug() << "点击了节点：" << text;

	// 判断当前的节点是树的第几个层级
	int level = getTreeItemLevel(item);
	switch (level) {
	case 0:
		// 树第一层级
		qDebug() << "树第一层级";
		break;
	case 1:
		// 树第二层级
		qDebug() << "树第二层级";
		break;
	case 2:
		// 树第三层级
		qDebug() << "树第三层级";
		break;
	case 3:
		// 树第四层级
		qDebug() << "树第四层级";
		break;
	default:
		break;
	}

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
