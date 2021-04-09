
#include <Esp.h>
#include <Wire.h> 
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pthread.h> 


#include <queue>
//#include "AsyncUDP.h"

#include <stdio.h>
#include <cstdlib>

const char * udpAddress = "192.168.43.255"; // broadcast


const char* ssid = "Robot";
const char* password =  "robot123";



#define UDP_PORT 9001

#define QUEUE_SIZE 100
#define MEMORY_BUFFOR_SIZE 200 

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


#define MSG_CONTROLER_RESPONSE 50



#define NOTIFICATION_LED_PIN_1 12
#define NOTIFICATION_LED_PIN_2 14


enum returnType{
  E_OK = 0,
  E_NOT_OK,
  E_NOT_AVAILABLE,
  E_PERIPHERAL_BROKEN,
  E_UNKNOWN_ERROR
};

enum SystemState{
  IDLE = 0,
  MEMORY_PLAY
};

static SystemState ESP_STATE = IDLE; 

struct WirelessData{
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


class NotificationLED{
  bool isOn = false; 
  bool isEnabled = false; 
  bool isBlinking = false; 
  unsigned long lastCall = 0 ; 
  uint8_t myPin = 0; 
  uint16_t myDelayTime = 0; 
  uint16_t myDelayBackup = 0; 
  uint8_t myShortDelayTime = 50; 
  public: 
  void check(){
    if(isEnabled && isBlinking)
    {
      if(myDelayBackup != 0) 
      {
        if((millis() - lastCall) >= myDelayTime)
        {
          blink(); 
          lastCall = millis(); 
        }
      }
    }
  }

  void setup(uint8_t pin, uint16_t delayTime)
  {
    myPin = pin; 
    myDelayBackup = delayTime;
  }

  void blink(){
    if(isOn)
    {
      digitalWrite(myPin, LOW);
      myDelayTime = myDelayBackup; 
      isOn = false;  
    }else
    {
      digitalWrite(myPin, HIGH);
      myDelayTime = myShortDelayTime; 
      isOn = true; 
    };
  }

  void setDelay(uint16_t newDelay){
    myDelayBackup = newDelay; 
  }

  void on(bool a_isBlinking)
  {
    digitalWrite(myPin, HIGH);
    this->isBlinking = a_isBlinking;
    isEnabled = true; 
  }

  void off()
  {
    digitalWrite(myPin, LOW);
    isEnabled = false; 
  }
};


class WirelessDataQueue{
  private:
  pthread_mutex_t lock; 
  WirelessData queue[QUEUE_SIZE]; 
  int index = 0; 
  uint8_t size = QUEUE_SIZE; 
  bool empty = true;
  public:
  WirelessDataQueue()
  {
    pthread_mutex_init(&lock, NULL);
    memset(&queue, 0 , sizeof(WirelessData)*QUEUE_SIZE);
  }

  bool isEmpty()
  {
    return empty;
  }

  bool isFull()
  {
    bool retVal;

    pthread_mutex_lock(&lock); 
    if(index == (QUEUE_SIZE-2))
    {
      retVal = true;
    }else
    {
      retVal = false;
    }
    pthread_mutex_unlock(&lock); 

    return retVal; 
  }

  uint8_t push(WirelessData cmd)
  {
    if(!isFull())
    {
      pthread_mutex_lock(&lock); 
      memcpy(&queue[index], &cmd, sizeof(WirelessData));
      index++; 
      pthread_mutex_unlock(&lock); 
      empty = false; 
      return 0;
    }else
    {
      return 1;
    }
  }

  uint8_t pop(WirelessData* cmd)
  {
    if(!isEmpty())
    {
      pthread_mutex_lock(&lock); 
      memcpy(cmd, &queue[0], sizeof(WirelessData));
      for(int i =0; i<index ; i++)
      {
        memcpy(&queue[i], &queue[i+1], sizeof(WirelessData));
      }
      index -- ; 
      if(index == 0)
      {
        index = 0 ; 
        empty = true; 
      }
      pthread_mutex_unlock(&lock); 
      return 0;
    }else
    {
      return 1; 
    }
  }
};


NotificationLED Notification_LED1; 
NotificationLED Notification_LED2; 
//AsyncUDP udpsocket;
WiFiUDP udpsocket;
pthread_mutex_t socketMutex; 


//Wifi cyclic messages listening thread
pthread_t WiFiListeningThread; 
//pthread_t WiFiStatusResponseThread; UNUSED 
pthread_t ThreadRouter; 

uint8_t transmiti2c(WirelessData cmd, uint8_t DeviceAdress);
void udpSend(WirelessData cmd);
void wifihandler();
uint8_t getMessageReceiver(uint8_t MessageIdentifier);
WirelessData packDeviceResponse(); 
WirelessData getRememberedCommand();
bool isMemoryIndexValid(uint8_t index);
uint8_t checksumCalculate(byte* data); 
int checksumValidate(WirelessData data); 
void handleRecordStack(WirelessData data, uint64_t timeDelay);
returnType espCommandHandler(WirelessData cmd);

void* wifiThreadFunction(void * arg); 
void* messagesRouting(void * arg); 
//void* wifiStatusResponseThreadFunction(void * arg);  # unused

/* Received messages */ 
WirelessDataQueue MessagesToBeProcessed; 

/* Messages waiting for routing to slaves */ 
WirelessDataQueue MessagesToBeRouted; 

/* ESP transmited status */ 
WirelessDataQueue MessagesToBeTransmited; 

std::queue<WirelessData>* ToTransmitSTDQueue; 

/* MOVE RECORDING */
typedef struct{
  WirelessData command; 
  uint64_t timeDelayToNext; 
} RememberedCommand; 
RememberedCommand RememberedCommandArray[MEMORY_BUFFOR_SIZE]; 
static uint8_t recordingMemory = 0; 
static bool isRecordingOn = false; 
static uint64_t lastMsgTime = 0; 
static uint64_t lastMemoryMsgTime = 0; 
static uint8_t currentReadingPosition = 0; 
/* MOVE RECORDING */
 

void setup() {
  Serial.begin(9600);

  Wire.begin(); 

  pinMode(NOTIFICATION_LED_PIN_1, OUTPUT); 
  Notification_LED1.setup(NOTIFICATION_LED_PIN_1, 700);
  Notification_LED1.on(true); 
  pinMode(NOTIFICATION_LED_PIN_2, OUTPUT);
  Notification_LED2.setup(NOTIFICATION_LED_PIN_2, 1000);
  Notification_LED2.on(true); 

  /* WiFi connection handle begin   */ 
  uint8_t iterator = 0; 
  int connectionCounter = 1; 

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    Notification_LED1.check();
    Notification_LED2.check();
    delay(50);
  }

  udpsocket.begin(UDP_PORT);
    /* WiFi connection handle end   */ 


  /* Start blinking green LED (LED2), wifi connected properly  */ 
  Notification_LED1.off();
  Notification_LED2.off();
  delay(100);

  /* System queues initialize  */ 
  MessagesToBeProcessed = WirelessDataQueue(); 
  MessagesToBeRouted = WirelessDataQueue(); 
  MessagesToBeTransmited = WirelessDataQueue(); 

  ToTransmitSTDQueue = new std::queue<WirelessData>; 

  /* Threads initialization, start blinking red LED when failed */ 
  int threadInit = pthread_create(&WiFiListeningThread, NULL, &wifiThreadFunction, NULL);
  if(0 != threadInit)
  {
    Notification_LED1.on(true); 
    Notification_LED1.setDelay(100); 
  }

  threadInit |= pthread_create(&ThreadRouter, NULL, &messagesRouting, NULL); 
  if(0 != threadInit)
  {
    Notification_LED1.on(true); 
    Notification_LED1.setDelay(100); 
  }


  for(uint8_t i = 0; i < MEMORY_BUFFOR_SIZE; i++)
  {
    RememberedCommandArray[i].command = {0}; 
    RememberedCommandArray[i].timeDelayToNext = 0; 
  }
  recordingMemory = 0; 
  isRecordingOn = false; 
}


unsigned long time1 = 0; 

void loop() {
  if(ESP_STATE == MEMORY_PLAY)
  {
    WirelessData command = getRememberedCommand();
    if(command.MessageIdentifier != 0)
    {
      MessagesToBeProcessed.push(command);
    }
  }

  Notification_LED1.check();
  Notification_LED2.check();

  delay(10); 
}


uint8_t transmiti2c(WirelessData cmd, uint8_t DeviceAdress)
{
  uint8_t retVal; 
  byte* bytesToSend = (byte*)malloc(sizeof(cmd));
  memcpy(bytesToSend, &cmd, sizeof(cmd)); 

  Wire.beginTransmission(DeviceAdress); // transmit to device 
  for(uint8_t i = 0; i < sizeof(cmd); i++)
  {
    Wire.write((byte)(bytesToSend[i]));     
  }
  retVal = Wire.endTransmission();    // stop transmitting
  Serial.println("Transmission finished with status : "); 
  Serial.println(retVal);
  free(bytesToSend); 
  return retVal; 
}

void udpSend(WirelessData cmd)
{
  udpsocket.beginPacket(udpAddress, UDP_PORT);

  udpsocket.write((uint8_t*)&cmd, sizeof(cmd));

  udpsocket.endPacket();
} 

void wifihandler()
{
  /* received messages queue is not empty, cmd has to be handled */ 
  if(! MessagesToBeProcessed.isEmpty())
  {
    WirelessData recvPacket; 
    memset(&recvPacket, 0 , sizeof(WirelessData));
    /* Get command from queue */ 
    if(0 == MessagesToBeProcessed.pop(&recvPacket))
    {
      /* Push received message to routing queue */ 
      MessagesToBeRouted.push(recvPacket);      
    }
  }
}


returnType espCommandHandler(WirelessData cmd)
{
  returnType retVal = E_OK; 
  switch(cmd.MessageIdentifier){
    case MSG_ESP_START_RECORDING:
      isRecordingOn = true; 
      break; 
    case MSG_ESP_STOP_RECORDING:
      isRecordingOn = false; 
      break; 
    case MSG_ESP_PLAY_RECORDING:
      currentReadingPosition = 0; 
      ESP_STATE = MEMORY_PLAY; 
      break; 
    case MSG_ESP_CLEAR_RECORDING:
      recordingMemory = 0; 
      currentReadingPosition = 0; 
      for(uint8_t i = 0; i < MEMORY_BUFFOR_SIZE; i++)
      {
        RememberedCommandArray[i].command = {0}; 
        RememberedCommandArray[i].timeDelayToNext = 0; 
      }
      break; 

    case MSG_ESP_NOTIFICATION_LED1_DELAY:
      Notification_LED1.setDelay(cmd.Byte10);
      break; 
    case MSG_ESP_NOTIFICATION_LED2_DELAY:
      Notification_LED2.setDelay(cmd.Byte10);
      break; 
    
    default : break; 
  }

  return retVal;   
}


void* wifiThreadFunction(void * arg)
{
  while(true)
  {
    if(WiFi.status() == WL_CONNECTED)
    { 
      /* Response message is waiting to be transmitted */ 
      if(!MessagesToBeTransmited.isEmpty())
      {
        WirelessData message = {0}; 
        MessagesToBeTransmited.pop(&message); 
        //WirelessData message = ToTransmitSTDQueue->front(); 
        //ToTransmitSTDQueue->pop(); 
        udpSend(message); 
        delay(10);

        Notification_LED1.off(); 
      }
      
      /* command receive allowed while no playing memory */
      //if(ESP_STATE != MEMORY_PLAY)
      //{
        int packetSize = udpsocket.parsePacket();
        // Packet has been received 
        if (packetSize)
        {
          WirelessData recvPacket; 
          memset(&recvPacket, 0, sizeof(recvPacket)); 
          int len = udpsocket.read((char*)&recvPacket, sizeof(recvPacket));
          // Packet is correct length
          if(len >= sizeof(recvPacket))
          {
            if(ESP_STATE != MEMORY_PLAY)
            {
              // Push packet to command queue
              Notification_LED2.on(false); 
              MessagesToBeProcessed.push(recvPacket);
            }else if(recvPacket.MessageIdentifier == MSG_ESP_PLAY_INTERRUPT)
            {
              ESP_STATE = IDLE; 
            }
            
          }
        } 
      //}
    }
    usleep(50000);
  }
}

uint8_t getMessageReceiver(uint8_t MessageIdentifier)
{
  uint8_t retVal = 0; 
  if(MessageIdentifier < 10)
  {
    retVal = DEVICE_ID_PLATFORM; 
  }else if(MessageIdentifier < 20)
  {
    retVal = DEVICE_ID_ARM; 
  }else
  {
    retVal = DEVICE_ID_ESP32; 
  }
  return retVal;
}; 


void* messagesRouting(void * arg)
{
  WirelessData recvPacket; 
  uint16_t timeToRespondCounter = 0; 
  while(true)
  {
    timeToRespondCounter++; 
    if(timeToRespondCounter >= 3 /* 100ms */)
    {
      Notification_LED1.on(false); 
      /* request slaves for data, pack them to udp message, push message to queue - wifi thread will handle sending */ 
      MessagesToBeTransmited.push(packDeviceResponse()); 
      //ToTransmitSTDQueue->push(packDeviceResponse());
      timeToRespondCounter = 0; 
    }

    if(! MessagesToBeProcessed.isEmpty())
    {
      memset(&recvPacket, 0 , sizeof(WirelessData));
      /* Get command from queue */ 
      if(0 == MessagesToBeProcessed.pop(&recvPacket))
      {
        uint8_t receiver = getMessageReceiver(recvPacket.MessageIdentifier);
        if(DEVICE_ID_ESP32 != receiver)
        {
          if(0 != transmiti2c(recvPacket, receiver))
          {
            Notification_LED1.on(true); 
            Notification_LED1.setDelay(50); 
          }else
          {
            Notification_LED1.off(); 
          }

          /*Command shall be recorded*/
          if(isRecordingOn)
          {

            handleRecordStack(recvPacket, (millis()-lastMsgTime));
          }

          lastMsgTime = millis(); 
        }else
        {
          espCommandHandler(recvPacket);
        }
        Notification_LED2.off(); 
      }
    }
    usleep(63001);
  }
}

WirelessData packDeviceResponse()
{
  WirelessData respondMessage_ARM = {0}; /* filled by ARM */ 
  WirelessData respondMessage_PLATFORM = {0}; /* filled by platform */ 
  WirelessData udpResponse = {0}; /* two above merged into one */ 

  Wire.requestFrom(DEVICE_ID_ARM, sizeof(respondMessage_ARM)); 
  while(Wire.available())   
  { 
    Wire.readBytes((byte*)&respondMessage_ARM, sizeof(respondMessage_ARM)); 
  }

  Wire.requestFrom(DEVICE_ID_PLATFORM, sizeof(respondMessage_PLATFORM)); 
  while(Wire.available())   
  { 
    Wire.readBytes((byte*)&respondMessage_PLATFORM, sizeof(respondMessage_PLATFORM)); 
  }

  udpResponse.MessageIdentifier = MSG_CONTROLER_RESPONSE; 
  udpResponse.Byte1 = respondMessage_ARM.Byte1;
  udpResponse.Byte2 = respondMessage_ARM.Byte2;
  udpResponse.Byte3 = respondMessage_ARM.Byte3;
  udpResponse.Byte4 = respondMessage_ARM.Byte4;
  udpResponse.Byte5 = currentReadingPosition;
  
  udpResponse.Byte6 = respondMessage_PLATFORM.Byte1;
  udpResponse.Byte7 = respondMessage_PLATFORM.Byte2;
  udpResponse.Byte8 = respondMessage_PLATFORM.Byte3;
  udpResponse.Byte9 = respondMessage_PLATFORM.Byte4;

  udpResponse.Byte10 = recordingMemory; 
  return udpResponse; 
}


uint8_t checksumCalculate(byte* data)
{
  uint8_t checksum = 0; 
  for(uint8_t index = 0; index < sizeof(WirelessData); index++)
  {
    checksum += (uint8_t)(data[index] % 7); 
  }
  return checksum; 
} 

int checksumValidate(WirelessData data)
{
  int retVal = 0; 

  if(data.Byte10 != checksumCalculate((byte*)&data))
  {
    retVal = -1; 
  }

  return retVal; 
}

void handleRecordStack(WirelessData data, uint64_t timeDelay)
{
  if(recordingMemory < MEMORY_BUFFOR_SIZE)
  {
    RememberedCommandArray[recordingMemory].command = data; 

    if(0 != recordingMemory)
    {
      RememberedCommandArray[recordingMemory].timeDelayToNext = timeDelay; 
    }else
    {
      RememberedCommandArray[recordingMemory].timeDelayToNext = 0;
    }
    recordingMemory++; 
  }
}


WirelessData getRememberedCommand()
{
  if((currentReadingPosition < MEMORY_BUFFOR_SIZE) && (isMemoryIndexValid(currentReadingPosition)))
  {
    if(currentReadingPosition == 0)
    {
      currentReadingPosition ++; 
      lastMemoryMsgTime = millis(); 
      return RememberedCommandArray[0].command; 
    }else
    {
      if((millis() - lastMemoryMsgTime) >= RememberedCommandArray[currentReadingPosition].timeDelayToNext )
      {
        WirelessData cmd = RememberedCommandArray[currentReadingPosition].command;
        currentReadingPosition++; 
        lastMemoryMsgTime = millis(); 
        return cmd; 
      }else
      {
        return {0};
      }
    }
  }else
  {
    if(ESP_STATE == MEMORY_PLAY)
    {
      currentReadingPosition = 0; 
      ESP_STATE = IDLE; 
    }
    return {0}; 
  }
}

bool isMemoryIndexValid(uint8_t index)
{
  if(RememberedCommandArray[index].command.MessageIdentifier != 0)
  {
    return true; 
  }else
  {
    return false; 
  }
}
