#include "GUIRenderer.h"
#define CLICK_OUT_OF_RANGE 1
#define CLICK_ARM_AREA 0 
#define CLICK_MOTOR_AREA 2


#define NEVER_CONNECTED 0
#define CONNECTED 1 
#define CONNECTION_LOST 2 
GUIRenderer::GUIRenderer(int a_startX, int a_startY, int a_xLength, int a_yLength)
{
    this->startX = a_startX;
    this->startY = a_startY;
    this->xLength = a_xLength;
    this->yLength = a_yLength;

    servo1Angle =120;
    servo2Angle = 80;
    servo3Angle = 140;
	servo4Angle = 30; 
    pointMarked = true;
    pointX = 10;
    pointY = 10;
	ClawLevel = 0; 

	slider1.setParams(140, 350, 150, "RIGHT / FRONT:");
	slider2.setParams(140, 420, 150, "RIGHT / REAR:");
	slider3.setParams(140, 490, 150, "LEFT / FRONT:");
	slider4.setParams(140, 560, 150, "LEFT / REAR:");

	motor1Pwm = slider1.value + 100; 
	motor2Pwm = slider2.value + 100;
	motor3Pwm = slider3.value + 100;
	motor4Pwm = slider4.value + 100;
	recordBarValue = 0; 

}


void GUIRenderer::plot(QPainter* p_Painter)
{
	QFont font = p_Painter->font();
	int backup = font.pointSize();
	font.setPointSize(font.pointSize() * 1.6);
	p_Painter->setFont(font);
	QString pwm = "Motors PWM control";
	p_Painter->drawText(180, 300, pwm);
	font.setPointSize(backup);
	p_Painter->setFont(font);
	p_Painter->drawLine(10, 680, 1540, 680);

	plotClawBar(p_Painter);

    GraphPen.setColor(Qt::gray);
    GraphPen.setWidth(1);
    GraphPen.setStyle(Qt::DotLine);
    p_Painter->setPen(GraphPen);
	plotClawBar(p_Painter);

    plotOXgrid(p_Painter);
    plotOYgrid(p_Painter);
    plotXYAxis(p_Painter);

    plotArm1(p_Painter);
    plotArm2(p_Painter);
    plotArm3(p_Painter);


    if(pointMarked)
    {
        GraphPen.setColor(Qt::blue);
        GraphPen.setWidth(10);
        GraphPen.setStyle(Qt::DotLine);
        p_Painter->setPen(GraphPen);
        p_Painter->drawPoint(pointX, pointY);
    }


	GraphPen.setColor(Qt::darkBlue);
	p_Painter->setPen(GraphPen);
	font.setPointSize(font.pointSize() * 2);
	p_Painter->setFont(font);
	p_Painter->drawText(70, 50, "Arm angles: "); 
	p_Painter->drawText(300, 50, "Motors PWM: ");
	p_Painter->drawText(600, 710, "RECORDING PANEL ");

	font.setPointSize(backup);
	p_Painter->setFont(font);
	font.setPointSize(font.pointSize() * 1.6);
	p_Painter->setFont(font);
	p_Painter->drawText(800, 780, "Recording memory ");

     p_Painter->drawText(90, 100, "Angle1 : " + QString::number(servo1Angle));
     p_Painter->drawText(90, 150, "Angle2 : " + QString::number(servo2Angle));
     p_Painter->drawText(90, 200, "Angle3 : " + QString::number(servo3Angle));
	 p_Painter->drawText(90, 250, "Angle4 : " + QString::number(servo4Angle));


	 p_Painter->drawText(300, 100, "RIGHT/FRONT: " + QString::number(motor1Pwm));
	 p_Painter->drawText(300, 150, "RIGHT/REAR : " + QString::number(motor2Pwm));
	 p_Painter->drawText(300, 200, "LEFT/FRONT : " + QString::number(motor3Pwm));
	 p_Painter->drawText(300, 250, "LEFT/REAR : " + QString::number(motor4Pwm));
	 




	font.setPointSize(backup);
	p_Painter->setFont(font);


	 slider1.draw(p_Painter, &GraphPen);
	 slider2.draw(p_Painter, &GraphPen);
	 slider3.draw(p_Painter, &GraphPen);
	 slider4.draw(p_Painter, &GraphPen);

	 
	 switch (connectionStatus)
	 {
	 case NEVER_CONNECTED:
		 GraphPen.setColor(Qt::darkRed);
		 p_Painter->setPen(GraphPen);
		 font.setPointSize(font.pointSize() * 2);
		 p_Painter->setFont(font);
		 p_Painter->drawText(850, 70, "Device not connected");
		 break;
	 case CONNECTED:
		 GraphPen.setColor(Qt::green);
		 p_Painter->setPen(GraphPen);
		 font.setPointSize(font.pointSize() * 2);
		 p_Painter->setFont(font);
		 p_Painter->drawText(850, 70, "Connection OK");
		 break;
	 case CONNECTION_LOST:
		 GraphPen.setColor(Qt::red);
		 p_Painter->setPen(GraphPen);
		 font.setPointSize(font.pointSize() * 2);
		 p_Painter->setFont(font);
		 p_Painter->drawText(850, 70, "Connection lost!");
		 break; 
	 }
	 font.setPointSize(backup);
	 p_Painter->setFont(font);

	 plotRecordBar(p_Painter);


	 if (isLastMoveInvalid)
	 {
		 GraphPen.setColor(Qt::red);
		 p_Painter->setPen(GraphPen);
		 font.setPointSize(font.pointSize() * 4);
		 p_Painter->setFont(font);
		 p_Painter->drawText(800, 350, "Invalid move");
	 }

}


void GUIRenderer::plotOXgrid(QPainter* p_Painter)
{
    /* Grid X */
    int GridXPoint = startX;
    int xCoord = 0;
    while(GridXPoint < (startX + xLength))
    {
        /* Draw vertical grid  */
        GraphPen.setColor(Qt::gray);
        GraphPen.setWidth(1);
        GraphPen.setStyle(Qt::DotLine);
        p_Painter->setPen(GraphPen);
        p_Painter->drawLine(GridXPoint, (startY+(int)(0.4*yLength)), GridXPoint, startY-yLength);

        /* Draw coordinates  */
        QString str = QString::number(xCoord);
        GraphPen.setColor(Qt::black);
        GraphPen.setWidth(2);
        GraphPen.setStyle(Qt::SolidLine);
        p_Painter->setPen(GraphPen);
        p_Painter->drawText(GridXPoint-5, startY+20, str);
        xCoord ++;

        GridXPoint +=20;
    }
}


void GUIRenderer::plotOYgrid(QPainter* p_Painter)
{
    /* Grid Y */
    int GridYPoint = startY+(int)(0.4*yLength);
    int yCoord = 0;
    while(GridYPoint >= (startY - yLength))
    {
        /* Draw horizontal grid  */
        GraphPen.setColor(Qt::gray);
        GraphPen.setWidth(1);
        GraphPen.setStyle(Qt::DotLine);
        p_Painter->setPen(GraphPen);
        p_Painter->drawLine(startX, GridYPoint, startX+xLength, GridYPoint);


        /* Draw coordinates  */
        if(GridYPoint <= startY+(int)(0.4*yLength))
        {
            GraphPen.setColor(Qt::black);
            GraphPen.setWidth(2);
            GraphPen.setStyle(Qt::SolidLine);
            p_Painter->setPen(GraphPen);
            QString str = QString::number(yCoord);
            p_Painter->drawText(startX-20, GridYPoint+5, str);
            yCoord++;
        }


        GridYPoint -=20;
    }
}


void GUIRenderer::plotXYAxis(QPainter* p_Painter)
{
    GraphPen.setColor(Qt::black);
    GraphPen.setWidth(3);
    p_Painter->setPen(GraphPen);

    /* X axis */
    p_Painter->drawLine(startX,startY,startX+xLength,startY);
    /* arrow */
    p_Painter->drawLine(startX+xLength,startY, (startX+xLength)-((int)(xLength*0.02)),startY+(int)(yLength*0.02));
    p_Painter->drawLine(startX+xLength,startY, (startX+xLength)-((int)(xLength*0.02)),startY-(int)(yLength*0.02));

    /* Y axis */
    p_Painter->drawLine(startX,startY,startX,startY-yLength);
    /* arrow */
    p_Painter->drawLine(startX,startY-yLength, startX-((int)(xLength*0.02)),(startY-yLength)+(int)(yLength*0.04));
    p_Painter->drawLine(startX,startY-yLength, startX+((int)(xLength*0.02)),(startY-yLength)+(int)(yLength*0.04));

}


void GUIRenderer::plotArm1(QPainter * p_Painter)
{
    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(10);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawPoint(startX, startY);

    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(3);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawLine(startX,
                        startY,
                        (startX+cos((servo1Angle*PI) / 180) * (ARM1_LENGTH*20)),
                        (startY-sin((servo1Angle*PI) / 180) * (ARM1_LENGTH*20)));

}

void GUIRenderer::plotArm2(QPainter * p_Painter)
{
    int myStartX = startX+cos((servo1Angle*PI) / 180) * (ARM1_LENGTH*20);
    int myStartY = startY-sin((servo1Angle*PI) / 180) * (ARM1_LENGTH*20);
    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(10);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawPoint(myStartX, myStartY);


    int angle = servo2Angle - (180 - servo1Angle);

    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(3);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawLine(myStartX,
                        myStartY,
                        (myStartX+cos((angle*PI) / 180) * (ARM2_LENGTH*20)),
                        (myStartY-sin((angle*PI) / 180) * (ARM2_LENGTH*20)));

}


void GUIRenderer::plotArm3(QPainter * p_Painter)
{
    int myStartX = startX+cos((servo1Angle*PI) / 180) * (ARM1_LENGTH*20);
    int myStartY = startY-sin((servo1Angle*PI) / 180) * (ARM1_LENGTH*20);
    int prevangle = servo2Angle - (180 - servo1Angle);
    myStartX+=cos((prevangle*PI) / 180) * (ARM2_LENGTH*20);
    myStartY-=sin((prevangle*PI) / 180) * (ARM2_LENGTH*20);


    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(10);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawPoint(myStartX, myStartY);


    int angle = servo3Angle - (180 - servo2Angle) - (180 - servo1Angle);
    GraphPen.setColor(Qt::darkGreen);
    GraphPen.setWidth(3);
    GraphPen.setStyle(Qt::SolidLine);
    p_Painter->setPen(GraphPen);
    p_Painter->drawLine(myStartX,
                        myStartY,
                        (myStartX+cos((angle*PI) / 180) * (ARM3_LENGTH*20)),
                        (myStartY-sin((angle*PI) / 180) * (ARM3_LENGTH*20)));
}


int GUIRenderer::graphClickHandle(int x, int y)
{
	int retVal = CLICK_OUT_OF_RANGE; 
    if((x > startX) && (x <= startX+xLength) &&
       (y < startY+(int)(0.4*yLength)) && (y>= startY-yLength))
    {
		retVal = CLICK_ARM_AREA;
        /* get closest x grid coord */
        int xGridCoord = startX + ((int)((x - startX) / 20))*20;

        /* get closest y grid coord */
        int yGridCoord = startY - ((int)((startY - y) / 20))*20;

       if(!pointMarked)
       {
            pointMarked = true;
       }
       this->pointX = xGridCoord;
       this->pointY = yGridCoord;


       solver.getAnglesAlgorithm(((int)((pointX - startX) / 20)), ((int)((startY - pointY)/20)), &servo1Angle, &servo2Angle, &servo3Angle);


    }else
    {
        pointMarked = false;
    }

	if ((x > 1380) && (x <= 1420) &&
		(y <= 500) && (y >= 100))
	{
		retVal = CLICK_ARM_AREA;
		ClawLevel = 500 - y; 
		servo4Angle = 20 + (int)((80.0 / 500.0) * (ClawLevel));
	}


	if (true == slider1.wasClicked(x, y, &motor1Pwm))
	{
		retVal = CLICK_MOTOR_AREA;
	}

	if (true == slider2.wasClicked(x, y, &motor2Pwm))
	{
		retVal = CLICK_MOTOR_AREA;
	}

	if (true == slider3.wasClicked(x, y, &motor3Pwm))
	{
		retVal = CLICK_MOTOR_AREA;
	}

	if (true == slider4.wasClicked(x, y, &motor4Pwm))
	{
		retVal = CLICK_MOTOR_AREA;
	}

	return retVal; 
}


void GUIRenderer::plotClawBar(QPainter* painter)
{
	QFont font = painter->font();
	int backup = font.pointSize(); 
	font.setPointSize(font.pointSize() * 1.2); 
	painter->setFont(font); 
	GraphPen.setColor(Qt::black);
	GraphPen.setStyle(Qt::SolidLine);
	painter->setPen(GraphPen);
	QString str1 = "CLENCH";
	QString str2 = "RELEASE";
	painter->drawText(1375, 90, str1); 
	painter->drawText(1375, 520, str2);
	font.setPointSize(backup);
	painter->setFont(font);

	GraphPen.setColor(Qt::gray);
	GraphPen.setWidth(3);
	GraphPen.setStyle(Qt::DotLine);
	painter->setPen(GraphPen);
	painter->drawLine(1400, 100, 1400, 500); 



	GraphPen.setColor(Qt::yellow);
	GraphPen.setWidth(5);
	GraphPen.setStyle(Qt::SolidLine);
	painter->setPen(GraphPen);
	painter->drawLine(1380, 500-ClawLevel, 1420, 500-ClawLevel);

}


void GUIRenderer::resetPWM()
{
	motor1Pwm = 180; 
	motor2Pwm = 180;
	motor3Pwm = 180;
	motor4Pwm = 180;

	slider1.value = motor1Pwm - 100; 
	slider2.value = motor2Pwm - 100;
	slider3.value = motor3Pwm - 100;
	slider4.value = motor4Pwm - 100;
}


void GUIRenderer::plotRecordBar(QPainter* painter)
{
	GraphPen.setColor(Qt::gray);
	GraphPen.setWidth(2);
	GraphPen.setStyle(Qt::SolidLine);
	painter->setPen(GraphPen);
	painter->drawLine(500, 800, 1300, 800);
	painter->drawLine(500, 810, 1300, 810);
	painter->drawLine(500, 800, 500, 810);
	painter->drawLine(1300, 800, 1300, 810);

	GraphPen.setColor(Qt::white);
	GraphPen.setWidth(8);
	painter->setPen(GraphPen);
	painter->drawLine(505, 805, 1295, 805);
	GraphPen.setColor(Qt::green);
	painter->setPen(GraphPen);
	GraphPen.setWidth(5);
	int progressBarEnd = 500 + (4 * recordBarValue);

	if (progressBarEnd > 1295)
	{
		progressBarEnd = 1295;
	}
	if (recordBarValue != 0)
	{
		painter->drawLine(505, 805, progressBarEnd, 805);
	}
	GraphPen.setColor(Qt::black);
	painter->setPen(GraphPen);
	painter->drawText(740, 830, "(Actual playing index) / (Remembered commands number)");
	painter->drawText(850, 855, QString::number(currentlyPlaying) + " / " + QString::number(recordBarValue));


}
