#pragma once
#include <qpainter.h>
class HorizontalSlider
{
public:
	HorizontalSlider();
	~HorizontalSlider();
	void setParams(int x, int y, int range, QString name);
	void draw(QPainter* painter, QPen* GraphPen);
	bool wasClicked(int x, int y, int* valuePtr);
	int getValue; 
	int value; 
private:
	int xStart;
	int yStart;
	int range;
	QString name; 
};

