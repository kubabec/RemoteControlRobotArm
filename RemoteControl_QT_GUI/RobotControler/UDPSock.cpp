#include "UDPSock.h"
#include "mainwindow.h"
#define MSG_SLAVE_RESPONSE 50
#define port 9001
class MainWindow; 

UDPSock::UDPSock(MainWindow *parent)
	: QObject(parent)
{
	myParent = parent;
	socket = new QUdpSocket(this); 
	socket->bind(port);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	

	//messagesSenderThread = new std::thread(&UDPSock::threadSendingFunction, this);
}

UDPSock::~UDPSock()
{
}

void UDPSock::sendMessage(WirelessData msg)
{
	QByteArray Data;
	Data.append((char*) &msg, sizeof(msg));
	int udpsuccess = socket->writeDatagram(Data, QHostAddress::Broadcast, port);
}


void UDPSock::readyRead() {
	QByteArray Buffer; 
	Buffer.resize(socket->pendingDatagramSize());
	QHostAddress sender;
	quint16 senderPort; 
	socket->readDatagram(Buffer.data(), Buffer.size(), &sender, &senderPort); 
	if (Buffer.size() >= sizeof(WirelessData))
	{
		WirelessData receivedMsg = { 0 };
		memcpy(&receivedMsg, Buffer.data(), sizeof(receivedMsg));
		if (MSG_SLAVE_RESPONSE == receivedMsg.MessageIdentifier)
		{
			myParent->msgRecvCallback(receivedMsg, false);
		}
	}
}


void UDPSock::send(WirelessData data)
{
	//messagesToBeSentQueue.push(data);
	sendMessage(data);
}


void UDPSock::threadSendingFunction()
{
	//while (true)
	//{
	//	while (!messagesToBeSentQueue.empty())
	//	{
	//		WirelessData cmd = messagesToBeSentQueue.front();
	//		messagesToBeSentQueue.pop();
			
	//	}
		
	//	Sleep(20);
	//}
}