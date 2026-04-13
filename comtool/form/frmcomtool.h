#ifndef FRMCOMTOOL_H
#define FRMCOMTOOL_H

#include <QWidget>
#include "qtcpsocket.h"
#include "qtcpserver.h"
#include "Modbus.h"
#include "serialdataqueue.h"
#include "serialworker.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include "serial_protocol_parser.h"
#include <functional>

#include <CommunicationProtocol.h>
#include <InfraredSpectrumData.h>
#include <ImageData.h>


namespace Ui
{
	class frmComTool;
}

class frmComTool : public QWidget
{
	Q_OBJECT

public:
	explicit frmComTool(QWidget* parent = 0);
	// 初始化树形控件
	void initTreeWidget();
	void parseXmlToTree(const QString& xmlData);
	void addTreeNode(QTreeWidgetItem* parentItem, const QString& nodeName);
	~frmComTool();

private:
	Ui::frmComTool* ui;

	//bool comOk;                 //串口是否打开
	//QextSerialPort* com;        //串口通信对象
	//QTimer* timerRead;          //定时读取串口数据
	//QTimer* timerSend;          //定时发送串口数据
	//QTimer* timerSave;          //定时保存串口数据
	//QTimer* timerReLoad;		//重新上电；
	int reLoadTimes = 10;		// 重连次数
	// 业务状态
	int m_state = 0;   // 0正常待机状态 ，1：进入检测完成状态，2：进入待机状态 ，3：进入结果状态 

	int sleepTime;              //接收延时时间
	int sendCount;              //发送数据计数
	int receiveCount;           //接收数据计数
	bool isShow;                //是否显示数据

	//bool tcpOk;                 //网络是否正常
	//QTcpSocket* socket;         //网络连接对象
	QTimer* timerConnect;       //定时器重连

	SerialProtocolParser* parser;

	QTreeWidgetItem* fileItem;
	QTreeWidgetItem* imageItem;
	QTreeWidgetItem* mCurrentItem;

	CommunicationProtocol m_protocol;
private:
	SerialDataQueue* m_sendQueue;
	SerialDataQueue* m_receiveQueue;
	SerialWorker* m_serialWorker;
private slots:
	void initForm();            //初始化窗体数据
	void initConfig();          //初始化配置文件
	void saveConfig();          //保存配置文件
	void readData(uint8_t type);            //读取串口数据
	void sendData();            //发送串口数据
	void sendData(std::vector<uint8_t> data);
	void sendData(const QByteArray& data);
	void reLoad();

	void changeEnable(bool b);  //改变状态
	//void append(int type, const QString& data, bool clear = false);

	QStringList enumerateSerialPorts();
	void processReceivedData();
	void openSerialPort();
	void closeSerialPort();
	int getTreeItemLevel(QTreeWidgetItem* item);

	void addTreeConmunicationItem();
	void addData2TreeCommunication();
	void addTreeItemImageData(QTreeWidgetItem* parentItem);
	void addData2TreeImageData();
    void addTreeItemInfraredSpectrumData(QTreeWidgetItem* parentItem);
	void addData2TreeInfraredSpectrumData(QTreeWidgetItem* parent);
	QTreeWidgetItem* addChildItem(QTreeWidgetItem* parent, const QString& name, const QString& type);
	// 根据参数名称查找节点（第一列）
	QTreeWidgetItem* findItemByName(const QString& name);
	QTreeWidgetItem* createEditableItem(QTreeWidgetItem* parent);
	void packed_data_received(const std::vector<uint8_t>& data);
	

	QString getPacketTypeName(uint32_t packetTypeCode);
	QString formatXmlString(const std::string& str);
	//QByteArray qt_gzip_decompress(const QByteArray& gzipData);
	//std::vector<uint8_t> qt_gzip_compress(std::vector<uint8_t> gzipData);
	std::vector<uint8_t> qt_gzip_load_file(std::string filepath);
	bool  qt_gzip_save_file(const std::string& fileName, const std::vector<uint8_t>& data);
	bool  qt_gzip_save_file(const std::string& fileName, const std::vector<uint8_t>& data,bool append);
	bool zip_mem_compress(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files, std::vector<uint8_t>& zip_data);

private slots:
	void onSerialError(const QString& error);
	void onSerialPortOpened();
	void onSerialPortClosed();

private slots:
	void on_btnOpen_clicked();
	void on_cboxPortName_clicked(int nindex);
	void on_btnReceiveCount_clicked();

	void on_pushButton_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_4_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked(); // 插入图片

	void onTreeItemClicked(QTreeWidgetItem* item, int column);
	
};

#endif // FRMCOMTOOL_H
