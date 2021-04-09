#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QtGui"
#include "QtCore"
#include "GUIRenderer.h"

#define CLICK_OUT_OF_RANGE 1
#define CLICK_ARM_AREA 0 
#define CLICK_MOTOR_AREA 2
#define NEVER_CONNECTED 0
#define CONNECTED 1 
#define CONNECTION_LOST 2 

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->renderer = new GUIRenderer(700,500,600,400);
	this->socketSender = new UDPSock(this);
	TimeSinceLastKey = 0; 
	Timer.start(); 



	engine1Enable = new QCheckBox("ON", this);
	engine1Enable->setGeometry(QRect(QPoint(500, 325),
		QSize(50, 50)));
	engine2Enable = new QCheckBox("ON", this);
	engine2Enable->setGeometry(QRect(QPoint(500, 395),
		QSize(50, 50)));
	engine3Enable = new QCheckBox("ON", this);
	engine3Enable->setGeometry(QRect(QPoint(500, 465),
		QSize(50, 50)));
	engine4Enable = new QCheckBox("ON", this);
	engine4Enable->setGeometry(QRect(QPoint(500, 535),
		QSize(50, 50)));

	engine1Enable->setCheckState(Qt::Unchecked);
	engine2Enable->setCheckState(Qt::Unchecked);
	engine3Enable->setCheckState(Qt::Unchecked);
	engine4Enable->setCheckState(Qt::Unchecked);

	m_button = new QPushButton("reset PWM", this);
	// set size and location of the button
	m_button->setGeometry(QRect(QPoint(50, 600),
		QSize(200, 50)));

	// Connect button signal to appropriate slot
	connect(m_button, SIGNAL(released()), this, SLOT(handleButton()));


	fullSpeedButton = new QPushButton("Full power", this);
	// set size and location of the button
	fullSpeedButton->setGeometry(QRect(QPoint(300, 600),
		QSize(200, 50)));

	// Connect button signal to appropriate slot
	connect(fullSpeedButton, SIGNAL(released()), this, SLOT(handleFullSpeed()));

	/*MOVE RECORDING BUTTONS */
	startRecordingButton = new QPushButton("START recording", this);
	QPalette pal = startRecordingButton->palette();
	pal.setColor(QPalette::Button, QColor(Qt::blue));
	startRecordingButton->setAutoFillBackground(true);
	startRecordingButton->setPalette(pal);
	// set size and location of the button
	startRecordingButton->setGeometry(QRect(QPoint(100, 730),
		QSize(150, 50)));
	connect(startRecordingButton, SIGNAL(released()), this, SLOT(startRecording()));


	stopRecordingButton = new QPushButton("STOP recording", this);
	QPalette pal1 = stopRecordingButton->palette();
	pal.setColor(QPalette::Button, QColor(Qt::red));
	stopRecordingButton->setAutoFillBackground(true);
	stopRecordingButton->setPalette(pal);
	// set size and location of the button
	stopRecordingButton->setGeometry(QRect(QPoint(300, 730),
		QSize(150, 50)));
	connect(stopRecordingButton, SIGNAL(released()), this, SLOT(stopRecording()));

	playButton = new QPushButton("Play", this);
	QPalette pal2 = playButton->palette();
	pal.setColor(QPalette::Button, QColor(Qt::green));
	playButton->setAutoFillBackground(true);
	playButton->setPalette(pal);
	// set size and location of the button
	playButton->setGeometry(QRect(QPoint(100, 800),
		QSize(150, 50)));
	connect(playButton, SIGNAL(released()), this, SLOT(playRecording()));

	pauseButton = new QPushButton("Pause", this);
	QPalette pal3 = pauseButton->palette();
	pal.setColor(QPalette::Button, QColor(Qt::blue));
	pauseButton->setAutoFillBackground(true);
	pauseButton->setPalette(pal3);
	// set size and location of the button
	pauseButton->setGeometry(QRect(QPoint(300, 800),
		QSize(150, 50)));
	connect(pauseButton, SIGNAL(released()), this, SLOT(pauseButtonHandler()));

	clearRecording = new QPushButton("Clear recording", this);
	// set size and location of the button
	clearRecording->setGeometry(QRect(QPoint(1330, 780),
		QSize(150, 50)));
	connect(clearRecording, SIGNAL(released()), this, SLOT(clearRecordMemory()));

	/*MOVE RECORDING BUTTONS */


	// Send motor enable value to device
	connect(engine1Enable, SIGNAL(clicked()), this, SLOT(checkboxClick()));
	connect(engine2Enable, SIGNAL(clicked()), this, SLOT(checkboxClick()));
	connect(engine3Enable, SIGNAL(clicked()), this, SLOT(checkboxClick()));
	connect(engine4Enable, SIGNAL(clicked()), this, SLOT(checkboxClick()));

	sendNewPwmRequest();
	messageTimeoutHandler = new std::thread(&MainWindow::timeoutHandleFunction, this);

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    renderer->plot(&painter);
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    int graphClickValidation = renderer->graphClickHandle((int)event->localPos().x(), (int)event->localPos().y());
    QString str = "mouse click: ";
    str+= QString::number(event->localPos().x());
    str+= ", ";
    str+= QString::number(event->localPos().y());

	if (CLICK_ARM_AREA == graphClickValidation)
	{
		readAnglesFromrenderer();
	}
	else if (CLICK_MOTOR_AREA == graphClickValidation)
	{
		readPwmFromrenderer(); 
	}
	//repaint();
}

void MainWindow::readAnglesFromrenderer()
{
	servo1Angle = renderer->servo1Angle;
	servo2Angle = renderer->servo2Angle;
	servo3Angle = renderer->servo3Angle;
	servo4Angle = renderer->servo4Angle;
	sendNewAnglesRequest();
}

void MainWindow::readPwmFromrenderer()
{
	motor1Pwm = renderer->motor1Pwm; 
	motor2Pwm = renderer->motor2Pwm;
	motor3Pwm = renderer->motor3Pwm;
	motor4Pwm = renderer->motor4Pwm;
	sendNewPwmRequest(); 
}


void MainWindow::sendNewAnglesRequest()
{
	if (servo1Angle != -1 &&
		servo2Angle != -1 &&
		servo3Angle != -1)
	{
		renderer->isLastMoveInvalid = false;
		WirelessData cmd = { 0 };
		cmd.MessageIdentifier = MSG_ARM_POSITION;
		cmd.Byte1 = servo1Angle; 
		cmd.Byte2 = servo2Angle;
		cmd.Byte3 = servo3Angle; 
		cmd.Byte4 = servo4Angle; 
		socketSender->send(cmd);
	}
	else
	{
		renderer->isLastMoveInvalid = true; 
	}
}

void MainWindow::sendNewPwmRequest()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_PLATFORM_PWM_CONTROL;
	if (engine1Enable->isChecked())
	{
		cmd.Byte1 = motor1Pwm;
	}

	if (engine2Enable->isChecked())
	{
		cmd.Byte2 = motor2Pwm;
	}

	if (engine3Enable->checkState())
	{
		cmd.Byte3 = motor3Pwm;
	}

	if (engine4Enable->isChecked())
	{
		cmd.Byte4 = motor4Pwm;
	}

	socketSender->send(cmd);
}


void MainWindow::keyPressEvent(QKeyEvent *keyEvent)
{
	
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_PLATROFM_CONTROL;
	switch (keyEvent->key())
	{
	case Qt::Key_W:
		cmd.Byte1 = RIDE_FORWARD;
		if ((Timer.elapsed() - TimeSinceLastKey) > 85)
		{
			socketSender->send(cmd);
			TimeSinceLastKey = Timer.elapsed();
		}
		break;
	case Qt::Key_S:
		cmd.Byte1 = RIDE_BACKWARD;
		if ((Timer.elapsed() - TimeSinceLastKey) > 85)
		{
			socketSender->send(cmd);
			TimeSinceLastKey = Timer.elapsed();
		}
		break;
	case Qt::Key_A:
		cmd.Byte1 = TURN_LEFT;
		if ((Timer.elapsed() - TimeSinceLastKey) > 85)
		{
			socketSender->send(cmd);
			TimeSinceLastKey = Timer.elapsed();
		}
		break;
	case Qt::Key_D: 
		cmd.Byte1 = TURN_RIGHT;
		if ((Timer.elapsed() - TimeSinceLastKey) > 85)
		{
			socketSender->send(cmd);
			TimeSinceLastKey = Timer.elapsed();
		}
		break;
	default: break;
	}
//	repaint();
		
	
}



uint8_t MainWindow::checksumCalculate(byte* data)
{
	uint8_t checksum = 0;
	for (uint8_t index = 0; index < sizeof(WirelessData); index++)
	{
		checksum += (uint8_t)(data[index] % 7);
	}
	return checksum;
}

int MainWindow::checksumValidate(WirelessData data)
{
	int retVal = 0;

	if (data.Byte10 != checksumCalculate((byte*)&data))
	{
		retVal = -1;
	}

	return retVal;
}



void MainWindow::handleButton()
{
	renderer->resetPWM();
	readPwmFromrenderer();
	//repaint(); 
}

void MainWindow::handleFullSpeed()
{

	/*WirelessData armLayDown = { 0 }; 
	armLayDown.MessageIdentifier = MSG_ARM_POSITION; 
	armLayDown.Byte1 = 160;
	armLayDown.Byte2 = 50; 
	armLayDown.Byte3 = 85; 
	armLayDown.Byte4 = 70; 
	socketSender->send(armLayDown);
	Sleep(5); */

	renderer->motor1Pwm = 250;
	renderer->slider1.value = 150; 
	renderer->motor2Pwm = 250;
	renderer->slider2.value = 150;
	renderer->motor3Pwm = 250;
	renderer->slider3.value = 150;
	renderer->motor4Pwm = 250;
	renderer->slider4.value = 150;
	readPwmFromrenderer();


	//repaint();
}

void MainWindow::checkboxClick()
{
	readPwmFromrenderer();
	//repaint();
}


/*RECORDING */

void MainWindow::startRecording()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_ESP_START_RECORDING;
	socketSender->send(cmd);
}
void MainWindow::stopRecording()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_ESP_STOP_RECORDING;
	socketSender->send(cmd);
}
void MainWindow::playRecording()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_ESP_PLAY_RECORDING;
	socketSender->send(cmd);
}
void MainWindow::clearRecordMemory()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_ESP_CLEAR_RECORDING;
	socketSender->send(cmd);
}

void MainWindow::pauseButtonHandler()
{
	WirelessData cmd = { 0 };
	cmd.MessageIdentifier = MSG_ESP_PLAY_INTERRUPT;
	socketSender->send(cmd);
}
/*RECORDING */

void MainWindow::msgRecvCallback(WirelessData data, bool errorOccurred)
{
	if (false == errorOccurred)
	{
		lastMsgTime = Timer.elapsed(); 
		messageNeverReceived = false; 
		renderer->connectionStatus = CONNECTED; 
		if ( (data.Byte1 > 0 && data.Byte2 > 0 && data.Byte3 > 0 && data.Byte4 > 0) &&
			(data.Byte1 < 180 && data.Byte2 < 180 && data.Byte3 < 180 && data.Byte4 < 180))
		{
			servo1Angle = data.Byte1 - 4;
			servo2Angle = data.Byte2;
			servo3Angle = data.Byte3;
			servo4Angle = data.Byte4;
			motor1Pwm = data.Byte6;
			motor2Pwm = data.Byte7;
			motor3Pwm = data.Byte8;
			motor4Pwm = data.Byte9;

			renderer->recordBarValue = data.Byte10;
			renderer->currentlyPlaying = data.Byte5;

			renderer->motor1Pwm = motor1Pwm;
			renderer->motor2Pwm = motor2Pwm;
			renderer->motor3Pwm = motor3Pwm;
			renderer->motor4Pwm = motor4Pwm;
			renderer->slider1.value = motor1Pwm - 100;
			renderer->slider2.value = motor2Pwm - 100;
			renderer->slider3.value = motor3Pwm - 100;
			renderer->slider4.value = motor4Pwm - 100;

			renderer->servo1Angle = servo1Angle;
			renderer->servo2Angle = servo2Angle;
			renderer->servo3Angle = servo3Angle;
			renderer->servo4Angle = servo4Angle;
			repaint();
		}

	}
	else
	{
		qDebug() << "last"; 
		renderer->connectionStatus = CONNECTION_LOST;
		repaint();
	}
	
}

void MainWindow::timeoutHandleFunction()
{
	while (true)
	{
		if (messageNeverReceived == false)
		{
			if (Timer.elapsed() - lastMsgTime >= 3000)
			{
				renderer->connectionStatus = CONNECTION_LOST; 
				update();
			}
		}

		Sleep(1000); 
	}
}

