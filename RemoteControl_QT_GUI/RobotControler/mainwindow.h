#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include "GUIRenderer.h"
#include "UDPSock.h"
#include <QMouseEvent>
#include <thread>
#include <queue> 
#include <QKeyEvent>
#include <QTime>
#include <QPushButton>
#include <QCheckBox>

// DEVICES 
#define DEVICE_ID_PLATFORM 0
#define DEVICE_ID_ARM 1
#define DEVICE_ID_ESP32 2
// DEVICES 

#define MSG_ESP_NOTIFICATION_LED1_DELAY 21 
#define MSG_ESP_NOTIFICATION_LED2_DELAY 22 
#define MSG_ESP_START_RECORDING 23
#define MSG_ESP_STOP_RECORDING 24
#define MSG_ESP_PLAY_RECORDING 25
#define MSG_ESP_CLEAR_RECORDING 26
#define MSG_ESP_PLAY_INTERRUPT 27

#define MSG_ARM_POSITION 11
#define MSG_ARM_NOTIFICATION_LED_DELAY 12 

#define MSG_PLATFORM_PWM_CONTROL 3 
#define MSG_PLATFORM_NOTIFICATION_LED_DELAY 2 
#define MSG_PLATROFM_CONTROL 1

#define MSG_SLAVE_RESPONSE 50


#define RIDE_FORWARD 1
#define RIDE_BACKWARD 2
#define TURN_LEFT 3
#define TURN_RIGHT 4
#define RIDE_STOP 0 

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    virtual void paintEvent(QPaintEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	void msgRecvCallback(WirelessData, bool errorOccurred);
    void mousePressEvent(QMouseEvent* event);
	void readAnglesFromrenderer(); 
	void readPwmFromrenderer();
	void sendNewAnglesRequest(); 
	void sendNewPwmRequest();
	QTime Timer;

private slots:
	void handleButton();
	void handleFullSpeed();
	void startRecording(); 
	void stopRecording(); 
	void playRecording();
	void clearRecordMemory(); 
	void checkboxClick();
	void pauseButtonHandler();

private:
	long long int TimeSinceLastKey; 
	UDPSock* socketSender; 
	int servo1Angle;
	int servo2Angle;
	int servo3Angle;
	int servo4Angle;
	int motor1Pwm;
	int motor2Pwm; 
	int motor3Pwm; 
	int motor4Pwm; 
    Ui::MainWindow *ui;
	GUIRenderer* renderer;
	int checksumValidate(WirelessData data);
	uint8_t checksumCalculate(byte* data);

	long long int lastMsgTime = 0; 
	bool messageNeverReceived = true; 
	std::thread* messageTimeoutHandler; 
	void timeoutHandleFunction();

	QPushButton *fullSpeedButton;
	QPushButton *m_button;
	QPushButton *startRecordingButton; 
	QPushButton *stopRecordingButton;
	QPushButton *playButton;
	QPushButton *clearRecording; 
	QPushButton *pauseButton; 

	QCheckBox *engine1Enable;
	QCheckBox *engine2Enable;
	QCheckBox *engine3Enable;
	QCheckBox *engine4Enable;

};
#endif // MAINWINDOW_H
