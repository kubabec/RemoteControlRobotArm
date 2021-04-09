#include <Servo.h>
#include <math.h> 
#include <Wire.h> 

#define NOTIFICATION_LED_PIN 8 

#define DEVICE_ID_PLATFORM 0
#define DEVICE_ID_ARM 1
#define DEVICE_ID_ESP32 2

#define MSG_ARM_POSITION 11
#define MSG_ARM_NOTIFICATION_LED_DELAY 12 
#define MSG_SLAVE_RESPONSE 50


#define ARM_1_LENGTH 8 
#define ARM_2_LENGTH 6 
#define ARM_3_LENGTH 15 
#define PI 3.14159265

#define INCORRECT_ANGLE 255

#define MAX_ANGLE_1 150 
#define MIN_ANGLE_1 15
#define MAX_ANGLE_2 180
#define MIN_ANGLE_2 40
#define MAX_ANGLE_3 180
#define MIN_ANGLE_3 40 
#define MAX_ANGLE_4 80
#define MIN_ANGLE_4 0 


#define SERVO1_SAFE_ANGLE 135 
#define SERVO2_SAFE_ANGLE 115
#define SERVO3_SAFE_ANGLE 90
#define SERVO4_SAFE_ANGLE 50 

#define SERVO_1_ANGLE_FIX 4


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


class Servomechanism;

// BEGIN FUNCTIONS PROTOTYPES
bool hasAnglesChanged(); 
void reachAngles();
void receiveEvent(int );
void requestEvent(WirelessData responseStatus);
// END FUNCTIONS PROTOTYPES 

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

class Servomechanism{
public:
  Servo servo; 
  uint8_t MyNumber; 
  uint8_t Step = 1; 
  uint8_t NewAngle = INCORRECT_ANGLE; 
  uint8_t CurrentAngle;
  uint8_t MaxAngle;
  uint8_t MinAngle; 
  uint8_t myAngleFix = 0; 

  void setup(uint8_t Number, uint8_t ControlPin, uint8_t StartingAngle, uint8_t maxAngle, uint8_t minAngle, uint8_t a_angleFix)
  {
        MyNumber = Number; 
        servo.attach(ControlPin); 
        myAngleFix = a_angleFix;
        CurrentAngle = StartingAngle; 
        servo.write(CurrentAngle); 
        MaxAngle = maxAngle; 
        MinAngle = minAngle; 
  }

  bool angleChanged()
  {
    if(NewAngle != (CurrentAngle-myAngleFix))
    {
        return true;
    }else
    {
        return false; 
    }
  }

  void reachAngle()
  {
      if(NewAngle != (CurrentAngle-myAngleFix))
      {
          if(NewAngle > (CurrentAngle-myAngleFix))
          {
            CurrentAngle += Step;    
          }else
          {
            CurrentAngle -= Step;     
          }
      }
  }

  void writeAngle()
  {
      servo.write(CurrentAngle); 
  }
};

Servomechanism servo1;
Servomechanism servo2;
Servomechanism servo3;
Servomechanism servo4;
NotificationLED Notification_LED; 

enum eSystemState{
  eWAITING_FOR_COMMAND = 0,
  eSETTING_POSITION,
  eFIRST_SETUP
};
eSystemState SystemState; 


bool hasAnglesChanged()
{
  if(servo1.angleChanged() || servo2.angleChanged() || servo3.angleChanged() || servo4.angleChanged())
  {
    return true;
  }else
  {
    return false; 
  }
}


void reachAngles()
{ 
  while(hasAnglesChanged())
  {
    /* Diode blink refresh */ 
    Notification_LED.check(); 

    /* Go throuth all servos and inc- or dec- rement angles if they are not equal to requested */ 
    if(servo1.angleChanged())
    {
      servo1.reachAngle();
      servo1.writeAngle();
    }
    if(servo2.angleChanged())
    {
      servo2.reachAngle();
      servo2.writeAngle();
    }
    if(servo3.angleChanged())
    {
      servo3.reachAngle();
      servo3.writeAngle();
    }    
    if(servo4.angleChanged())
    {
      servo4.reachAngle();
      servo4.writeAngle();
    }
    delay(20);
    
  }
}


void setup() {
    Wire.begin(DEVICE_ID_ARM);// join i2c bus with address #8
    Wire.onReceive(receiveEvent); // register event
    Wire.onRequest(requestEvent); // register event

    pinMode(NOTIFICATION_LED_PIN, OUTPUT);
    Notification_LED.setup(NOTIFICATION_LED_PIN, 100); 
    Notification_LED.off(); 
    pinMode(A0, INPUT); 

    servo1.setup(1, 11, 135, MAX_ANGLE_1, MIN_ANGLE_1, SERVO_1_ANGLE_FIX);
    servo2.setup(2, A1, 110, MAX_ANGLE_2, MIN_ANGLE_2, 0);
    servo3.setup(3, A2, 90, MAX_ANGLE_3, MIN_ANGLE_3, 0);
    servo4.setup(4, A3, 20, MAX_ANGLE_4, MIN_ANGLE_4, 0);
    SystemState = eFIRST_SETUP; 
    Serial.begin(9600);
}

int i = 0; 
void loop() {
    Notification_LED.check();
    
    switch(SystemState){
      case eFIRST_SETUP:
          Notification_LED.setDelay(100); 
          Notification_LED.on(true);
          // STARTUP PROCEDURE BEGIN 
          delay(1000);       
          servo1.NewAngle = 135; 
          servo2.NewAngle = 115; 
          servo3.NewAngle = 90; 
          servo4.NewAngle = 50; 
          reachAngles(); 
          delay(1000);

          servo1.NewAngle = 90; 
          servo2.NewAngle = 90; 
          servo3.NewAngle = 140; 
          servo4.NewAngle = 50; 
          reachAngles(); 
          delay(1000);    

          servo1.NewAngle = 135; 
          servo2.NewAngle = 115; 
          servo3.NewAngle = 90; 
          servo4.NewAngle = 50; 
          reachAngles(); 

          delay(1000); 
          SystemState = eWAITING_FOR_COMMAND; 
          // STARTUP PROCEDURE END 
          Serial.print("Setup finished. Ready to work. \n");
          Notification_LED.off(); 
          break; 
          
      case eWAITING_FOR_COMMAND: 
          Notification_LED.off(); 
          delay(10); 
          break;
          
      case eSETTING_POSITION:
          Serial.print("Setting position... \n"); 
          Notification_LED.setDelay(50); 
          reachAngles(); 

          
          SystemState = eWAITING_FOR_COMMAND;
          Notification_LED.setDelay(1000);
          
          break; 
      default: SystemState = eWAITING_FOR_COMMAND; break; 
    }
    
    delay(200);
}

void receiveEvent(int howMany) {
  // Command allowed only in proper state 
  if(SystemState == eWAITING_FOR_COMMAND)
  {
      unsigned int byteIndex = 0;
      WirelessData i2cMessage = {0};
      while (Wire.available()) { // loop through all but the last
        if(byteIndex < sizeof(i2cMessage)){
          uint8_t currentByte = Wire.read(); // receive byte as a character
          memcpy(((uint8_t*)&i2cMessage)+byteIndex, &currentByte, 1);
          byteIndex++;
        }
        else{
          unsigned char currentByte = Wire.read();
          Serial.print("WARNING: ignoring i2c byte\n");
        }
      }
      Serial.print("\n"); 
      if(byteIndex >= sizeof(i2cMessage))
      { 
        Notification_LED.on(false); 
        switch(i2cMessage.MessageIdentifier)
        {
          case MSG_ARM_POSITION:
            servo1.NewAngle = i2cMessage.Byte1; 
            servo2.NewAngle = i2cMessage.Byte2;
            servo3.NewAngle = i2cMessage.Byte3;
            servo4.NewAngle = i2cMessage.Byte4;
            SystemState = eSETTING_POSITION; 
            break;     
          
          case MSG_ARM_NOTIFICATION_LED_DELAY:
              Notification_LED.setDelay(i2cMessage.Byte1); 
            break;
            
          default:
            break;
        }
      }
      else{
        Serial.print("ERROR: command read failed, size to small\n");
      }
  }else
  {
    Serial.print("Unable to interrupt move! \n");
  }
  TWCR = 0;
  Wire.begin(DEVICE_ID_ARM);// join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  
}

void requestEvent()
{
  WirelessData slaveResponse = {0}; 
  slaveResponse.MessageIdentifier = MSG_SLAVE_RESPONSE; 
  slaveResponse.Byte1 = servo1.CurrentAngle; 
  slaveResponse.Byte2 = servo2.CurrentAngle; 
  slaveResponse.Byte3 = servo3.CurrentAngle;
  slaveResponse.Byte4 = servo4.CurrentAngle;
  slaveResponse.Byte5 = 0;
  Wire.write((byte*)&slaveResponse, sizeof(slaveResponse)); 
}
