#ifndef FRMCOMTOOL_H
#define FRMCOMTOOL_H

#include <QWidget>
#include "qtcpsocket.h"
#include "qtcpserver.h"
#include "Modbus.h"
#include "serialdataqueue.h"
#include "serialworker.h"

namespace Ui
{
	class frmComTool;
}

class frmComTool : public QWidget
{
	Q_OBJECT

public:
	explicit frmComTool(QWidget* parent = 0);
	~frmComTool();

private:
	Ui::frmComTool* ui;

	//bool comOk;                 //串口是否打开
	//QextSerialPort* com;        //串口通信对象
	//QTimer* timerRead;          //定时读取串口数据
	QTimer* timerSend;          //定时发送串口数据
	QTimer* timerSave;          //定时保存串口数据
	QTimer* timerReLoad;		//重新上电；
	int reloadtimes = 10;		// 重连次数
	// 业务状态
	int m_state = 0;   // 0正常待机状态 ，1：进入检测完成状态，2：进入待机状态 ，3：进入结果状态 

	int sleepTime;              //接收延时时间
	int sendCount;              //发送数据计数
	int receiveCount;           //接收数据计数
	bool isShow;                //是否显示数据

	//bool tcpOk;                 //网络是否正常
	//QTcpSocket* socket;         //网络连接对象
	QTimer* timerConnect;       //定时器重连

private:
	SerialDataQueue* m_sendQueue;
	SerialDataQueue* m_receiveQueue;
	SerialWorker* m_serialWorker;
private slots:
	void initForm();            //初始化窗体数据
	void initConfig();          //初始化配置文件
	void saveConfig();          //保存配置文件
	void readData();            //读取串口数据
	void readData(uint8_t type);            //读取串口数据
	void sendData();            //发送串口数据
	void sendData(QString data);//发送串口数据带参数
	void sendData(std::vector<uint8_t> data);
	void sendData(const QByteArray& data);
	void saveData();            //保存串口数据
	void reLoad();

	void changeEnable(bool b);  //改变状态
	void append(int type, const QString& data, bool clear = false);

	QStringList enumerateSerialPorts();
	void processReceivedData();
	void openSerialPort();
	void closeSerialPort();

	QString getPacketTypeName(uint32_t packetTypeCode);
	QString formatXmlString(const std::string& str);

private slots:
	void onSerialError(const QString& error);
	void onSerialPortOpened();
	void onSerialPortClosed();

private slots:
	void on_btnOpen_clicked();
	//void on_btnStopShow_clicked();
	void on_cboxPortName_clicked(int nindex);
	void on_btnSendCount_clicked();
	void on_btnReceiveCount_clicked();

	void on_pushButton_ReadStation_clicked();
	void on_pushButton_ReadBT_clicked();
	void on_pushButton_ReadTrigger_clicked();
	void on_pushButton_SetTrigger_clicked();
	void on_pushButton_SetTrigger_2_clicked();
	void on_pushButton_ReadState_clicked();
	void on_pushButton_ReadResult_clicked();
	void on_pushButton_ReadBattery_clicked();

	void on_pushButton_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_4_clicked();

	void on_btnClear_clicked();
	//void on_btnData_clicked();
	//void on_btnStart_clicked();

	void on_ckAutoSend_stateChanged(int arg1);
	void on_ckAutoSave_stateChanged(int arg1);
};

#endif // FRMCOMTOOL_H
