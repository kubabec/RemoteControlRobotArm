#ifndef GUIRenderer_H
#define GUIRenderer_H
#include <qpainter.h>
#include "AngleSolver.h"
#include "HorizontalSlider.h"

#define PI 3.14159265
#define ARM1_LENGTH 8
#define ARM2_LENGTH 6
#define ARM3_LENGTH 15
#define SERVO1_ANGLE_FIX 4
class GUIRenderer
{
public:
    GUIRenderer(int a_startX, int a_startY, int a_xLength, int a_yLength);
    void plot(QPainter* painter);
    int graphClickHandle(int x, int y);
	int recordBarValue = 0; 
	int currentlyPlaying = 0; 
	void resetPWM(); 
    int servo1Angle;
    int servo2Angle;
    int servo3Angle;
	int servo4Angle;

	int motor1Pwm;
	int motor2Pwm;
	int motor3Pwm;
	int motor4Pwm;

    bool pointMarked;
    int pointX;
    int pointY;

	HorizontalSlider slider1;
	HorizontalSlider slider2;
	HorizontalSlider slider3;
	HorizontalSlider slider4;

	int connectionStatus = 0 ;
	bool isLastMoveInvalid = false;

private:
   QPen GraphPen; 
   int startX;
   int startY;
   int xLength;
   int yLength;

   int ClawLevel; 
   AngleSolver solver;
   void plotOXgrid(QPainter* painter);
   void plotOYgrid(QPainter* painter);
   void plotXYAxis(QPainter* painter);

   void plotArm1(QPainter* painter);
   void plotArm2(QPainter* painter);
   void plotArm3(QPainter* painter);

   void plotClawBar(QPainter* painter);
   void plotRecordBar(QPainter* painter); 

};


#endif // GUIRenderer_H
