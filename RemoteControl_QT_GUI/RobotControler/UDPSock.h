#pragma once

#include <QObject>
#include <QMainWindow>
#include "QUdpSocket"
#include <queue>
#include <thread>
#include <windows.h> 
#include <qlabel.h>
#include <qnetworkdatagram.h>
class MainWindow;

struct WirelessData {
	uint8_t MessageIdentifier;
	uint8_t Byte1;
	uint8_t Byte2;
	uint8_t Byte3;
	uint8_t Byte4;
	uint8_t Byte5;
	uint8_t Byte6;
	uint8_t Byte7;
	uint8_t Byte8;
	uint8_t Byte9;
	uint8_t Byte10;
};

class UDPSock : public QObject
{
	Q_OBJECT

public:
	UDPSock(MainWindow *parent);
	~UDPSock();
	void send(WirelessData data);
	void sendMessage(WirelessData msg);
	std::queue<WirelessData> messagesToBeSentQueue;
	std::thread* messagesSenderThread;
	void threadSendingFunction();
signals: 

public slots: 
	void readyRead(); 

private:
	long long int lastMsgReceiveTime = 0;
	QUdpSocket *socket; 
	MainWindow* myParent;

	QLabel* labelToPrint; 
};
