#include <Wire.h>
#include <stdint.h>

#define PWM_DEFAULT 150 

#define NOTIFICATION_LED_PIN A0

#define DEVICE_ID_PLATFORM 0
#define DEVICE_ID_ARM 1
#define DEVICE_ID_ESP32 2

#define MSG_PLATFORM_PWM_CONTROL 3 
#define MSG_PLATFORM_NOTIFICATION_LED_DELAY 2 
#define MSG_PLATROFM_CONTROL 1
#define MSG_SLAVE_RESPONSE 50

#define RIDE_FORWARD 1
#define RIDE_BACKWARD 2
#define TURN_LEFT 3
#define TURN_RIGHT 4
#define RIDE_STOP 0 

#define MOTOR_DIR_FORWARD true
#define MOTOR_DIR_BACKWARD false
#define MOTOR_COUNT 4

#define WHEEL_1_PIN_1 2
#define WHEEL_1_PIN_2 4
#define WHEEL_1_PIN_PWM 3
#define WHEEL_2_PIN_1 8
#define WHEEL_2_PIN_2 7
#define WHEEL_2_PIN_PWM 5
#define WHEEL_3_PIN_1 9
#define WHEEL_3_PIN_2 12
#define WHEEL_3_PIN_PWM 6
#define WHEEL_4_PIN_1 10
#define WHEEL_4_PIN_2 13
#define WHEEL_4_PIN_PWM 11

#define FRONT_LIGHT_PIN A3
#define REAR_LIGHT_PIN A2

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

uint8_t lastknownmove = 0; 

enum ReturnType{
  E_OK = 0,
  E_NOT_OK
};

void receiveEvent(int );
void requestEvent();

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


class Wheel
{
 public:
    uint8_t speed = 0; 
    ReturnType setWheelPinOut(uint8_t a_pin1, uint8_t a_pin2, uint8_t a_pwm, bool a_isPolarizationReverse)
      {
        this->pin1 = a_pin1;
        this->pin2 = a_pin2;
        this->PinPWM = a_pwm; 

        this->IsPinoutSet = true;

        isPolarizationReverse = a_isPolarizationReverse;

        return E_OK;
      };

    bool checkIfPinoutSet()
      {
        return IsPinoutSet;
      };

    void forward()
    {
      if(isPolarizationReverse){
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH); 
      }
      else
      {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW); 
      }
    };

    void backward()
    {
      if(false == isPolarizationReverse){
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH); 
      }
      else
      {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW); 
      }
    };

    void stop()
    {
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, LOW); 
    //  analogWrite(PinPWM, 0);
    }

    void setSpeed(uint8_t speed)
    {
      this->speed = speed; 
      analogWrite(PinPWM, speed); 
    }

  private: 
    bool IsPinoutSet = false; 
    uint8_t pin1; 
    uint8_t pin2; 
    uint8_t PinPWM; 
    bool isPolarizationReverse;
};

Wheel Platform_Wheels_Array[MOTOR_COUNT]; 
uint8_t PWM_Value = 0; 

void platformStop()
{ 
  for(uint8_t i = 0; i < 4; i++)
  {
    Platform_Wheels_Array[i].stop();
  };
};


void setSpeedToAll(uint8_t speed)
{
  Platform_Wheels_Array[0].setSpeed(speed);
  Platform_Wheels_Array[1].setSpeed(speed);
  Platform_Wheels_Array[2].setSpeed(speed);
  Platform_Wheels_Array[3].setSpeed(speed);
}

void platformForward()
{
  Platform_Wheels_Array[0].forward();
  Platform_Wheels_Array[1].forward();
  
  Platform_Wheels_Array[2].forward();
  Platform_Wheels_Array[3].forward();

}

void platformBackward()
{
  Platform_Wheels_Array[0].backward();
  Platform_Wheels_Array[1].backward();
  
  Platform_Wheels_Array[2].backward();
  Platform_Wheels_Array[3].backward();
}


void platformRight()
{
  Platform_Wheels_Array[0].backward();
  Platform_Wheels_Array[1].backward();
  
  Platform_Wheels_Array[2].forward();
  Platform_Wheels_Array[3].forward();
}


void platformLeft()
{
  Platform_Wheels_Array[0].forward();
  Platform_Wheels_Array[1].forward();
  
  Platform_Wheels_Array[2].backward();
  Platform_Wheels_Array[3].backward();
}

NotificationLED Notification_LED; 
unsigned long lastRideRequestTime = 0 ; 

void setup() {
  Wire.begin(DEVICE_ID_PLATFORM);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event

  //setup serial for debug info
  Serial.begin(9600);
  Serial.print("INITIALIZED SUCCESSFULLY\n");

  /* Declare notification led pin and setup LED device */ 
  pinMode(NOTIFICATION_LED_PIN, OUTPUT);
  Notification_LED.setup(NOTIFICATION_LED_PIN, 2000); 

  /* Declare platform wheels pinout */ 
  pinMode(WHEEL_1_PIN_1, OUTPUT); 
  pinMode(WHEEL_1_PIN_2, OUTPUT); 
  pinMode(WHEEL_1_PIN_PWM, OUTPUT); 
  pinMode(WHEEL_2_PIN_1, OUTPUT); 
  pinMode(WHEEL_2_PIN_2, OUTPUT); 
  pinMode(WHEEL_2_PIN_PWM, OUTPUT); 
  pinMode(WHEEL_3_PIN_1, OUTPUT); 
  pinMode(WHEEL_3_PIN_2, OUTPUT); 
  pinMode(WHEEL_3_PIN_PWM, OUTPUT); 
  pinMode(WHEEL_4_PIN_1, OUTPUT); 
  pinMode(WHEEL_4_PIN_2, OUTPUT); 
  pinMode(WHEEL_4_PIN_PWM, OUTPUT);

  PWM_Value = PWM_DEFAULT; 

  /* Setup platform wheels */
  Platform_Wheels_Array[0].setWheelPinOut(WHEEL_1_PIN_1, WHEEL_1_PIN_2, WHEEL_1_PIN_PWM, true);
  Platform_Wheels_Array[1].setWheelPinOut(WHEEL_2_PIN_1, WHEEL_2_PIN_2, WHEEL_2_PIN_PWM, false);
  Platform_Wheels_Array[2].setWheelPinOut(WHEEL_3_PIN_1, WHEEL_3_PIN_2, WHEEL_3_PIN_PWM, true);
  Platform_Wheels_Array[3].setWheelPinOut(WHEEL_4_PIN_1, WHEEL_4_PIN_2, WHEEL_4_PIN_PWM, false);

  platformStop();
  
}

int timeToStop = 200; 
void loop() {
  switch(lastknownmove)
  {
    case RIDE_FORWARD:
      timeToStop = 120; 
      break;
    case RIDE_BACKWARD:
      timeToStop = 135; 
      break;
    default: 
      timeToStop = 150; 
      break; 
  }

  Notification_LED.off();
  if(millis() - lastRideRequestTime > timeToStop)
  {
    platformStop(); 
  }
  delay(5);
}


void receiveEvent(int howMany) {
  unsigned int byteIndex = 0;
  WirelessData i2cMessage = {0};
  while (Wire.available()) {
    if(byteIndex < sizeof(i2cMessage)){
      uint8_t currentByte = Wire.read(); // receive byte
      memcpy(((uint8_t*)&i2cMessage)+byteIndex, &currentByte, 1);
      byteIndex++;
    }
    else{
      unsigned char currentByte = Wire.read();
      Serial.print("WARNING: ignoring i2c byte\n");
    }
  }
  if(byteIndex >= sizeof(i2cMessage)){
    Notification_LED.on(false); 
    switch(i2cMessage.MessageIdentifier)
    {
      case MSG_PLATROFM_CONTROL:
        lastknownmove = i2cMessage.Byte1;
        switch(i2cMessage.Byte1){
          case RIDE_FORWARD:
            platformForward();
            break;
          case RIDE_BACKWARD:
            platformBackward();
            break;
          case TURN_LEFT:
            platformLeft(); 
            break;
          case TURN_RIGHT:
            platformRight(); 
            break; 
          case RIDE_STOP:
            platformStop();
            break;
          default: break; 
        }
        lastRideRequestTime = millis();
      break; 

      case MSG_PLATFORM_PWM_CONTROL:
        Platform_Wheels_Array[0].setSpeed(i2cMessage.Byte1);
        Platform_Wheels_Array[1].setSpeed(i2cMessage.Byte2);
        Platform_Wheels_Array[2].setSpeed(i2cMessage.Byte3);
        Platform_Wheels_Array[3].setSpeed(i2cMessage.Byte4);
        break; 

      case MSG_PLATFORM_NOTIFICATION_LED_DELAY:
        Notification_LED.setDelay(i2cMessage.Byte10);
        break; 

      default: break; 
    }
  }
  else{
    Serial.print("ERROR: command read failed, size to small\n");
  }

  TWCR = 0;
  Wire.begin(DEVICE_ID_PLATFORM);// join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
}
 

 void requestEvent()
{
  WirelessData slaveResponse = {0}; 
  slaveResponse.MessageIdentifier = MSG_SLAVE_RESPONSE; 
  slaveResponse.Byte1 = Platform_Wheels_Array[0].speed; 
  slaveResponse.Byte2 = Platform_Wheels_Array[1].speed; 
  slaveResponse.Byte3 = Platform_Wheels_Array[2].speed; 
  slaveResponse.Byte4 = Platform_Wheels_Array[3].speed; 
  Wire.write((byte*)&slaveResponse, sizeof(slaveResponse)); 
}