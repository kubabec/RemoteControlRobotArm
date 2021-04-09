# RemoteControlRobotArm

Project contains all files used to build remote control robot arm attached to wheeled platform. Device was created using 2x Arduino Uno and 1x ESP32. All of the microcontrollers are connected via I2C wire, where ESP32 is used as a master and Arduinos act as a slaves. ESP32 is a two-side control frames router, using WiFi connection and UDP packages. All of them are transmitted to the network broadcast address, so when more than one control GUI connected, all of them are able to real-time robot control. 

Video : https://www.youtube.com/watch?v=ky6azQb_MUg&ab_channel=KubaBecmer


![alt text](https://github.com/kubabec/RemoteControlRobotArm/blob/main/img/device.JPG?raw=true)

Robot is controled by QT application running on Windows. It allows user to platform drive (W/S/A/D) with each wheel power control and robotic arm control by coordinate system destination point clicking. User can also control device moves memory, which allows robot to remember few received commands and repeat them itself, e.g. autonomous object transfer (as on the video above). 

![alt text](https://github.com/kubabec/RemoteControlRobotArm/blob/main/img/QT_RC_GUI.JPG?raw=true)

GUI also provides real time information about robot arm angles with (real time update during move execution), moves memory load and power set on each wheel. It also detects connection issues when device is not present on the network. 


QT application contains AngleSolver class which is able to find correct servomechanisms angles (based on each arm part length), needed to reach requested XY axis point with end of the paw, so that entire arm control is done by one GUI click. 

Schematic part: 
![alt text](https://github.com/kubabec/RemoteControlRobotArm/blob/main/img/i2c_schematic.JPG?raw=true)
![alt text](https://github.com/kubabec/RemoteControlRobotArm/blob/main/img/servoArm_schematic.JPG?raw=true)
![alt text](https://github.com/kubabec/RemoteControlRobotArm/blob/main/img/wheeledPlatform_schematic.JPG?raw=true)
