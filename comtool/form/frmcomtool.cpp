#include "frmcomtool.h"
#include "ui_frmcomtool.h"
#include "qthelper.h"
#include "qthelperdata.h"

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
	fileItem = nullptr;
	imageItem = nullptr;
	mCurrentItem = nullptr;
	// 连接信号槽
	bool isConnected = connect(ui->treeWidget, &QTreeWidget::itemClicked,
		this, &frmComTool::onTreeItemClicked);
	qDebug() << "Connection successful:" << isConnected;

	// 自动调整列宽
	ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	ui->treeWidget_2->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui->treeWidget_2->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->treeWidget_2->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	//间隔颜色显示
	ui->treeWidget->setAlternatingRowColors(true);
	ui->treeWidget_2->setAlternatingRowColors(true);
	// 设置行高
	ui->treeWidget_2->setStyleSheet("QTreeWidget::item { height: 25px; }");

	addTreeConmunicationItem();
	addData2TreeCommunication();

	//QTreeWidgetItem*  = findItemByName("检测数据文件");
	//if (imageItem)
		//addTreeItemImageData(imageItem);

}

void frmComTool::parseXmlToTree(const QString& xmlData)
{
	QXmlStreamReader reader(xmlData);
	QTreeWidgetItem* currentParent = nullptr;

	int ntestpointcount = 0;

	// 逐行解析XML
	while (!reader.atEnd()) {
		if (reader.readNextStartElement()) {
			// 解析主任务
			if (reader.name() == "main_task") {
				QString name = reader.attributes().value("name").toString();
				currentParent = new QTreeWidgetItem(ui->treeWidget);
				currentParent->setText(0, name);
				currentParent->setExpanded(true); // 
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
				if (ntestpointcount == 0)
				{
					fileItem->setText(0, name);
					addTreeItemImageData(fileItem);
				}
				else
				{
					// 增加节点
					QTreeWidgetItem* targetItem = fileItem;
					QTreeWidgetItem* newItem = new QTreeWidgetItem();
					newItem->setText(0, name);
					if (QTreeWidgetItem* parent = targetItem->parent()) {
						// 获取目标节点在父节点里的索引
						int index = parent->indexOfChild(targetItem) + 1;
						// 插到它前面
						parent->insertChild(index, newItem);
					}
					else {
						// 顶层节点
						int index = ui->treeWidget_2->indexOfTopLevelItem(targetItem) + 1;
						// 获取目标节点的名称
						ui->treeWidget_2->insertTopLevelItem(index, newItem);
					}
					fileItem = newItem;
					addTreeItemImageData(fileItem);
				}

				ntestpointcount++;
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

	changeEnable(false);

	// 绑定组包完成信号
	parser = new SerialProtocolParser();
	parser->connect_packet_assembled(std::bind(&frmComTool::packed_data_received, this, std::placeholders::_1));

}

// 枚举可用串口的函数
QStringList frmComTool::enumerateSerialPorts()
{
	QStringList portList;

#ifdef Q_OS_WIN
	// Windows下通过查询注册表或试探法获取串口列表
	for (int i = 1; i <= 100; i++) {
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
			std::vector<uint8_t> dataVector(data.begin(), data.end());
			parser->parse_serial_data(dataVector);
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
	item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item1->setText(0, "报文头");
	item1->setText(1, "uint32_t (4字节)");

	// 2 版本号
	QTreeWidgetItem* item2 = new QTreeWidgetItem(ui->treeWidget_2);
	item2->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item2->setText(0, "版本号");
	item2->setText(1, "uint8_t (1字节)");
	//item2->setText(2, QString("0x%1").arg(m_protocol.version, 2, 16, QChar('0')).toUpper());

	// 3 序号
	QTreeWidgetItem* item3 = new QTreeWidgetItem(ui->treeWidget_2);
	item3->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item3->setText(0, "序号");
	item3->setText(1, "uint16_t (2字节)");
	//item3->setText(2, QString("0x%1").arg(m_protocol.sequenceNumber, 4, 16, QChar('0')).toUpper());


	// 4 请求标志
	QTreeWidgetItem* item4 = new QTreeWidgetItem(ui->treeWidget_2);
	item4->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item4->setText(0, "请求标志");
	item4->setText(1, "uint8_t (1字节)");
	//item4->setText(2, QString("0x%1").arg(m_protocol.requestFlag, 2, 16, QChar('0')).toUpper());

	// 5 数据包总长度
	QTreeWidgetItem* item5 = new QTreeWidgetItem(ui->treeWidget_2);
	item5->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item5->setText(0, "数据包总长度");
	item5->setText(1, "uint64_t (8字节)");
	//item5->setText(2, QString("0x%1").arg(m_protocol.totalPacketLength, 16, 16, QChar('0')).toUpper());

	// 6 报文类型编码
	QTreeWidgetItem* item6 = new QTreeWidgetItem(ui->treeWidget_2);
	item6->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item6->setText(0, "报文类型编码");
	item6->setText(1, "uint32_t (4字节)");
	//item6->setText(2, QString("0x%1").arg(m_protocol.packetTypeCode, 8, 16, QChar('0')).toUpper());

	// 7 压缩标志
	QTreeWidgetItem* item7 = new QTreeWidgetItem(ui->treeWidget_2);
	item7->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item7->setText(0, "压缩标志");
	item7->setText(1, "uint8_t (1字节)");
	//item7->setText(2, QString("0x%1").arg(m_protocol.compressionFlag, 2, 16, QChar('0')).toUpper());

	// 8 加密标志
	QTreeWidgetItem* item8 = new QTreeWidgetItem(ui->treeWidget_2);
	item8->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item8->setText(0, "加密标志");
	item8->setText(1, "uint8_t (1字节)");
	//item8->setText(2, QString("0x%1").arg(m_protocol.encryptionFlag, 2, 16, QChar('0')).toUpper());

	// 9 仪器厂商
	QTreeWidgetItem* item9 = new QTreeWidgetItem(ui->treeWidget_2);
	item9->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item9->setText(0, "仪器厂商");
	item9->setText(1, "uint8_t (1字节)");
	//item9->setText(2, QString("0x%1").arg(m_protocol.instrumentVendor, 2, 16, QChar('0')).toUpper());

	// 10 备用
	QTreeWidgetItem* item10 = new QTreeWidgetItem(ui->treeWidget_2);
	item10->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item10->setText(0, "备用");
	item10->setText(1, "uint8_t[15] (15字节)");
	//item10->setText(2, "0x00");

	// 11 业务数据格式
	QTreeWidgetItem* item11 = new QTreeWidgetItem(ui->treeWidget_2);
	item11->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item11->setText(0, "业务数据格式");
	item11->setText(1, "uint8_t (1字节)");
	//item11->setText(2, QString("0x%1").arg(m_protocol.serviceDataFormat, 2, 16, QChar('0')).toUpper());

	// 12 业务数据长度
	QTreeWidgetItem* item12 = new QTreeWidgetItem(ui->treeWidget_2);
	item12->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item12->setText(0, "业务数据长度");
	item12->setText(1, "uint64_t (8字节)");
	//item12->setText(2, QString("0x%1").arg(m_protocol.serviceDataLength, 16, 16, QChar('0')).toUpper());

	// 13 业务数据
	QTreeWidgetItem* item13 = new QTreeWidgetItem(ui->treeWidget_2);
	item13->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item13->setText(0, "业务数据");
	item13->setText(1, "uint8_t 数组 (动态)");
	//item13->setText(2, m_protocol.getServiceDataString().c_str());

	// 14 检测数据文件长度
	QTreeWidgetItem* item14 = new QTreeWidgetItem(ui->treeWidget_2);
	item14->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item14->setText(0, "检测数据文件长度");
	item14->setText(1, "uint64_t (8字节)");
	//item14->setText(2, QString("0x%1").arg(m_protocol.detectionFileLength, 16, 16, QChar('0')).toUpper());

	// 15 检测数据文件
	QTreeWidgetItem* item15 = new QTreeWidgetItem(ui->treeWidget_2);
	item15->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item15->setText(0, "检测数据文件");
	item15->setText(1, "uint8_t 数组 (动态)");
	fileItem = item15;

	// 16 校验字节
	QTreeWidgetItem* item16 = new QTreeWidgetItem(ui->treeWidget_2);
	item16->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item16->setText(0, "校验字节(CRC32)");
	item16->setText(1, "uint32_t (4字节)");
	//item16->setText(2, QString("0x%1").arg(m_protocol.crc32Checksum, 8, 16, QChar('0')).toUpper());

	// 17 报文尾
	QTreeWidgetItem* item17 = new QTreeWidgetItem(ui->treeWidget_2);
	item17->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item17->setText(0, "报文尾");
	item17->setText(1, "uint8_t (1字节)");
	//item17->setText(2, QString("0x%1").arg(m_protocol.packetTail, 2, 16, QChar('0')).toUpper());
}

void frmComTool::addData2TreeCommunication()
{
	QList<QTreeWidgetItem*> items = ui->treeWidget_2->findItems("报文头", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.packetHeader, 8, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("版本号", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.version, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("序号", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.sequenceNumber, 4, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("请求标志", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.requestFlag, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("数据包总长度", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.totalPacketLength, 16, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("报文类型编码", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.packetTypeCode, 8, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("压缩标志", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.compressionFlag, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("加密标志", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.encryptionFlag, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("仪器厂商", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.instrumentVendor, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("业务数据格式", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.serviceDataFormat, 2, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("业务数据长度", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.serviceDataLength, 16, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("业务数据", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, m_protocol.getServiceDataString().c_str());
	}
	items = ui->treeWidget_2->findItems("检测数据文件长度", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.detectionFileLength, 16, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("校验字节(CRC32)", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.crc32Checksum, 8, 16, QChar('0')).toUpper());
	}
	items = ui->treeWidget_2->findItems("报文尾", Qt::MatchExactly, 0);
	if (!items.isEmpty())
	{
		items.first()->setText(2, QString("0x%1").arg(m_protocol.packetTail, 2, 16, QChar('0')).toUpper());
	}
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
	imageItem = addChildItem(parentItem, "图谱数据", "动态长度");

	// 20 文件尾部预留
	addChildItem(parentItem, "文件尾部预留", "byte[32] (32字节)");

	// 21 CRC32
	addChildItem(parentItem, "CRC32", "int32 (4字节)");

	ImageData p;
	m_protocol.detectionFilesData.push_back(p);
}

void frmComTool::addData2TreeImageData()
{
	QList<QTreeWidgetItem*> mQtreeItemList;
	// 遍历树的子节点，如果其有子节点则保存到Qlist中
	for (size_t i = 0; i < ui->treeWidget_2->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->treeWidget_2->topLevelItem(i);
		if (item->childCount() != 0)
		{
			mQtreeItemList.append(item);
		}
	}
	int itemcout = 0;
	for (auto item : mQtreeItemList)
	{
		for (int i = 0; i < item->childCount(); i++)
		{
			QString itemName = item->child(i)->text(0);
			if (itemName == "文件长度 L")item->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(itemcout).fileLength, 8, 16, QChar('0')).toUpper());
			else if (itemName == "规范版本号")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getVersionString()));
			else if (itemName == "文件生成时间")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getCreateTimeString()));
			else if (itemName == "站点名称")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getStationNameString()));
			else if (itemName == "站点编码")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getStationCodeString()));
			else if (itemName == "天气")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getWeatherDescription()));
			else if (itemName == "温度")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).temperature));
			else if (itemName == "湿度")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).humidity));
			else if (itemName == "仪器厂家")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getInstrumentManufacturerString()));
			else if (itemName == "仪器型号")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getInstrumentModelString()));
			else if (itemName == "仪器版本号")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getInstrumentVersionString()));
			else if (itemName == "仪器序列号")item->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(itemcout).getInstrumentSerialNumberString()));
			else if (itemName == "系统频率")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).systemFrequency));
			else if (itemName == "图谱数量 N")item->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(itemcout).spectrumCount, 4, 16, QChar('0')).toUpper());
			else if (itemName == "经度")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).longitude));
			else if (itemName == "纬度")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).latitude));
			else if (itemName == "海拔")item->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(itemcout).altitude));
			else if (itemName == "CRC32")item->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(itemcout).crc32, 8, 16, QChar('0')).toUpper());
		}
		itemcout++;
	}

}

void frmComTool::addTreeItemInfraredSpectrumData(QTreeWidgetItem* parentItem)
{
	// 1. 检测数据类型编码
	addChildItem(parentItem, "检测数据类型编码", "uint8_t (1字节)");

	// 2. 图谱数据长度
	addChildItem(parentItem, "图谱数据长度", "uint32_t (4字节)");

	// 3. 图谱生成时间
	addChildItem(parentItem, "图谱生成时间", "uint64_t (8字节)");

	// 4. 图谱性质
	addChildItem(parentItem, "图谱性质", "uint8_t (1字节)");

	// 5. 被检测设备名称
	addChildItem(parentItem, "被检测设备名称", "char[118] (118字节)");

	// 6. 被检测设备编码
	addChildItem(parentItem, "被检测设备编码", "char[42] (42字节)");

	// 7. 测点名称
	addChildItem(parentItem, "测点名称", "char[128] (128字节)");

	// 8. 测点编码
	addChildItem(parentItem, "测点编码", "char[32] (32字节)");

	// 9. 检测通道标志
	addChildItem(parentItem, "检测通道标志", "int16_t (2字节)");

	// 10. 存储器数据类型
	addChildItem(parentItem, "存储器数据类型", "uint8_t (1字节)");

	// 11. 温度单位
	addChildItem(parentItem, "温度单位", "uint8_t (1字节)");

	// 12. 温度点阵宽度
	addChildItem(parentItem, "温度点阵宽度 w", "uint32_t (4字节)");

	// 13. 温度点阵高度
	addChildItem(parentItem, "温度点阵高度 h", "uint32_t (4字节)");

	// 14. 可见光照片数据长度
	addChildItem(parentItem, "可见光照片数据长度 L1", "uint32_t (4字节)");

	// 15. 红外照片数据长度
	addChildItem(parentItem, "红外照片数据长度 L2", "uint32_t (4字节)");

	// 16. 辐射率
	addChildItem(parentItem, "辐射率", "float (4字节)");

	// 17. 测试距离
	addChildItem(parentItem, "测试距离", "float (4字节)");

	// 18. 大气温度
	addChildItem(parentItem, "大气温度", "float (4字节)");

	// 19. 相对湿度
	addChildItem(parentItem, "相对湿度", "uint8_t (1字节)");

	// 20. 反射温度
	addChildItem(parentItem, "反射温度", "float (4字节)");

	// 21. 温宽上限
	addChildItem(parentItem, "温宽上限", "float (4字节)");

	// 22. 温宽下限
	addChildItem(parentItem, "温宽下限", "float (4字节)");

	// 23. 文件尾部预留
	addChildItem(parentItem, "文件尾部预留", "byte[133] (133字节)");

	// 24. 红外图谱数据
	addChildItem(parentItem, "红外图谱数据", "动态长度");

	// 25. 可见光照片数据
	addChildItem(parentItem, "可见光照片数据", "动态长度");

	// 26. 红外照片数据
	addChildItem(parentItem, "红外照片数据", "动态长度");
}

void frmComTool::addData2TreeInfraredSpectrumData(QTreeWidgetItem* parent)
{
	for (int i = 0; i < parent->childCount(); i++)
	{
		QString itemName = parent->child(i)->text(0);
		if (itemName == "检测数据类型编码")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).dataTypeCode, 2, 16, QChar('0')).toUpper());
		else if (itemName == "图谱数据长度")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).dataLength, 8, 16, QChar('0')).toUpper());
		else if (itemName == "图谱生成时间")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getCreateTimeString()));
		else if (itemName == "图谱性质")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getSpectrumPropertyDescription()));
		else if (itemName == "被检测设备名称")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getDeviceNameString()));
		else if (itemName == "被检测设备编码")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getDeviceCodeString()));
		else if (itemName == "测点名称")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getMeasurementPointNameString()));
		else if (itemName == "测点编码")parent->child(i)->setText(2, QString("%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).getMeasurementPointCodeString()));
		else if (itemName == "检测通道标志")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).detectionChannelFlag, 4, 16, QChar('0')).toUpper());
		else if (itemName == "存储器数据类型")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).storageDataType, 2, 16, QChar('0')).toUpper());
		else if (itemName == "温度单位")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).temperatureUnit, 2, 16, QChar('0')).toUpper());
		else if (itemName == "温度点阵宽度 w")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).temperatureMatrixWidth, 8, 16, QChar('0')).toUpper());
		else if (itemName == "温度点阵高度 h")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).temperatureMatrixHeight, 8, 16, QChar('0')).toUpper());
		else if (itemName == "可见光照片数据长度 L1")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).visibleLightDataLength, 8, 16, QChar('0')).toUpper());
		else if (itemName == "红外照片数据长度 L2")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).infraredPhotoDataLength, 8, 16, QChar('0')).toUpper());
		else if (itemName == "辐射率")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).emissivity));
		else if (itemName == "测试距离")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).testDistance));
		else if (itemName == "大气温度")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).atmosphericTemperature));
		else if (itemName == "相对湿度")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).relativeHumidity, 2, 16, QChar('0')).toUpper());
		else if (itemName == "反射温度")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).reflectedTemperature));
		else if (itemName == "温宽上限")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).temperatureRangeUpper));
		else if (itemName == "温宽下限")parent->child(i)->setText(2, QString::number(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).temperatureRangeLower));
		//else if (itemName == "文件尾部预留")parent->child(i)->setText(2, QString("0x%1").arg(m_protocol.detectionFilesData.at(0).infraredSpectrumData.at(0).fileTailReserved, 8, 16, QChar('0')).toUpper());
		else if (itemName == "红外图谱数据")parent->child(i)->setText(2, QString(""));
		else if (itemName == "可见光图谱数据")parent->child(i)->setText(2, QString(""));
		else if (itemName == "红外照片数据")parent->child(i)->setText(2, QString(""));
	}
}

// 工具函数：添加二级子节点
QTreeWidgetItem* frmComTool::addChildItem(QTreeWidgetItem* parent, const QString& name, const QString& type)
{
	QTreeWidgetItem* item = createEditableItem(parent);
	item->setText(0, name);
	item->setText(1, type);
	//item->setText(2, "输入参数");
	item->setExpanded(true); //
	return item;
}


QTreeWidgetItem* frmComTool::createEditableItem(QTreeWidgetItem* parent)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(parent);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	return item;
}

void frmComTool::onSerialPortOpened()
{
	changeEnable(true);
	ui->btnOpen->setText("关闭串口");
	sendData(Modbus::Modbus_Read_Power());
}

void frmComTool::onSerialPortClosed()
{
	changeEnable(false);
	ui->btnOpen->setText("打开串口");
}

void frmComTool::onSerialError(const QString& error)
{
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

	AppConfig::writeConfig();
}

void frmComTool::changeEnable(bool b)
{
	ui->cboxBaudRate->setEnabled(!b);
	ui->cboxDataBit->setEnabled(!b);
	ui->cboxParity->setEnabled(!b);
	ui->cboxPortName->setEnabled(!b);
	ui->cboxStopBit->setEnabled(!b);
}

void frmComTool::readData(uint8_t type)
{
	int count = 0;
	do
	{
		count++;
		QtHelper::sleep(sleepTime);
		QByteArray data;// = com->readAll();
		int dataLen = data.length();
		if (dataLen <= 0) {
			return;
		}

		if (isShow) {
			QString buffer;
			receiveCount = receiveCount + data.size();
		}
	} while (count < 100);
}

void  frmComTool::on_pushButton_clicked()
{
	QString strData = ui->textEdit->toPlainText();
	// 使用您提供的十六进制字符串
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
		m_protocol = protocol;
		addData2TreeCommunication();
	}
	else
	{
		qDebug() << "解析失败";
	}
}

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
// qt_gzip_save_file 可以向文件中追加数据
bool frmComTool::qt_gzip_save_file(const std::string& fileName, const std::vector<uint8_t>& data, bool append)
{
	if (append)
	{
		QFile file(fileName.c_str());
		if (!file.open(QIODevice::Append))
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
	else
	{
		return qt_gzip_save_file(fileName, data);
	}
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

	std::vector<std::vector<uint8_t>> dataList = parser->split_long_data(data, 250);

	for (auto item : dataList)
	{
		sendData(item);
		//将数据保存成dat文件
		qt_gzip_save_file("D:/test.dat", item, true);
	}
	qt_gzip_save_file("D:/test1.dat", data);
}

void frmComTool::on_pushButton_3_clicked()
{
	std::string DataConfer = "eb90eb9001000101000000000000003c8000000101000000000000000000000000000000000001000000000000000000000000000000007d0d3e0603";
	CommunicationProtocol protocol;
	if (protocol.buildFromHexString(DataConfer))
	{
		std::vector<std::vector<uint8_t>> mSendData = parser->split_long_data(protocol.toBytes(), 250);
		for (auto item : mSendData)
		{
			sendData(item);
			qDebug() << "发送数据成功" << item.size();
		}
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

void frmComTool::on_pushButton_6_clicked()
{
	// 弹出输入对话框，获取一个图像节点名称
	bool ok;
	QString fileName = QInputDialog::getText(this, "输入节点名称", "请输入图像节点名称：", QLineEdit::Normal, "", &ok);
	if (!ok || fileName.isEmpty())
	{
		return;
	}
	// 向 mCurrentItem 增加子节点
	QTreeWidgetItem* currentParent = new QTreeWidgetItem(mCurrentItem);

	// 判断当前mCurrentItem是否有子节点
	if (mCurrentItem->childCount() == 0)
	{
		imageItem->setText(0, fileName);
		addTreeItemInfraredSpectrumData(imageItem);
	}
	else
	{
		// 增加节点
		QTreeWidgetItem* targetItem = imageItem;
		QTreeWidgetItem* newItem = new QTreeWidgetItem();
		newItem->setText(0, fileName);
		if (QTreeWidgetItem* parent = targetItem->parent()) {
			// 获取目标节点在父节点里的索引
			int index = parent->indexOfChild(targetItem) + 1;
			// 插到它前面
			parent->insertChild(index, newItem);
		}
		else {
			// 顶层节点
			int index = ui->treeWidget_2->indexOfTopLevelItem(targetItem) + 1;
			// 获取目标节点的名称
			ui->treeWidget_2->insertTopLevelItem(index, newItem);
		}
		imageItem = newItem;
		addTreeItemInfraredSpectrumData(imageItem);
	}
	currentParent->setText(0, fileName);
	InfraredSpectrumData p;
	m_protocol.detectionFilesData.at(0).infraredSpectrumData.push_back(p);//TODO:at(0)与选中的mCurrentItem序号绑定
	addData2TreeInfraredSpectrumData(imageItem);
}

QTreeWidgetItem* frmComTool::findItemByName(const QString& name)
{
	// 遍历所有一级节点
	for (int i = 0; i < ui->treeWidget_2->topLevelItemCount(); i++) {
		QTreeWidgetItem* item = ui->treeWidget_2->topLevelItem(i);
		qDebug() << item->text(0);
		if (item->text(0) == name) {
			return item; // 找到返回
		}
	}
	return nullptr; // 没找到
}

void frmComTool::packed_data_received(const std::vector<uint8_t>& data)
{
	qDebug() << "收到数据包：" << QByteArray::fromRawData(reinterpret_cast<const char*>(data.data()), data.size()).toHex();

	//CommunicationProtocol protocol;

	if (m_protocol.buildFromBytes(data))
	{
		if (m_protocol.packetTypeCode == 0x00000001)
		{
			parseXmlToTree(formatXmlString(m_protocol.getServiceDataString()));
		}
		addData2TreeCommunication();
		addData2TreeImageData();
	}
	else
	{
		qDebug() << "CommunicationProtocol::buildFromBytes() failed";
	}
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
		//timerSend->setInterval(AppConfig::SendInterval);
		m_state = 0;
	}

}



void frmComTool::sendData(std::vector<uint8_t> data)
{
	//std::vector<uint8_t> 转成QTypeBuffer
	QByteArray buffer1;
	buffer1.append((char*)data.data(), data.size());
	////QByteArray buffer;
	if (m_sendQueue) {
		m_sendQueue->enqueueSendData(buffer1);
	}

	//将data 转成字符串
	QString buffer = QtHelperData::vectorToHexStr(data);

	sendCount = sendCount + data.size();
}

void frmComTool::sendData(const QByteArray& data)
{
	if (m_sendQueue) {
		m_sendQueue->enqueueSendData(data);
	}
}


void frmComTool::reLoad()
{
	if (reLoadTimes > 0) {
		reLoadTimes--;
		return;
	}
	reLoadTimes = 10;
}

void frmComTool::on_btnOpen_clicked()
{
	if (ui->btnOpen->text() == "打开串口") {
		openSerialPort();
	}
	else {
		closeSerialPort();
		changeEnable(false);
		ui->btnOpen->setText("打开串口");
	}
}

void frmComTool::on_btnReceiveCount_clicked()
{
	receiveCount = 0;
	//ui->btnReceiveCount->setText("接收 : 0 字节");
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
		mCurrentItem = item;
		break;
	default:
		break;
	}

}




