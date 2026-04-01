// serialworker.h
#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QThread>
#include "qextserialport.h"
#include "serialdataqueue.h"

class SerialWorker : public QThread
{
    Q_OBJECT

public:
    explicit SerialWorker(SerialDataQueue *sendQueue, SerialDataQueue *receiveQueue, QObject *parent = nullptr);
    ~SerialWorker();
    
    void setPortSettings(const QString &portName, BaudRateType baudRate, 
                        DataBitsType dataBits, ParityType parity, 
                        StopBitsType stopBits, FlowType flowControl);
    
    void stop(); // 停止线程

signals:
    void errorOccurred(const QString &error);
    void portOpened();
    void portClosed();

protected:
    void run() override;

private:
    void processSending();
    void processReceiving();
    
    volatile bool m_stopFlag;
    SerialDataQueue *m_sendQueue;
    SerialDataQueue *m_receiveQueue;
    
    QString m_portName;
    PortSettings m_portSettings;
    
    QextSerialPort *m_serialPort;
    QMutex m_portMutex;

    uint8_t functionCode;
};

#endif // SERIALWORKER_H#pragma once
