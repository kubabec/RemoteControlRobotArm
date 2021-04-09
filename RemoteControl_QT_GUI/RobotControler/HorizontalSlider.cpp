#include "HorizontalSlider.h"

#include <QDebug>

HorizontalSlider::HorizontalSlider()
{
	value = 70; 
}

void HorizontalSlider::setParams(int x, int y, int range, QString name)
{
	xStart = x;
	yStart = y;
	this->range = range;
	this->name = name; 
}

void HorizontalSlider::draw(QPainter* painter, QPen* GraphPen)
{
	GraphPen->setColor(Qt::gray);
	GraphPen->setStyle(Qt::DotLine);
	GraphPen->setWidth(3);
	painter->setPen(*GraphPen);
	painter->drawLine(xStart, yStart, xStart + (2 * range), yStart);

	GraphPen->setColor(Qt::yellow);
	GraphPen->setStyle(Qt::SolidLine);
	GraphPen->setWidth(3);
	painter->setPen(*GraphPen);
	painter->drawLine(xStart+(2 * value), yStart-25, xStart + (2* value), yStart+25);
	GraphPen->setColor(Qt::black);
	painter->setPen(*GraphPen);

	QString str1 = "100";
	QString str2 = "250";
	painter->drawText(xStart-30, yStart+5, str1);
	painter->drawText(xStart+10+ (2 * range), yStart+5, str2);

	QString str3 = QString::number(100+value);
	painter->drawText(xStart-10 + (value * 2), yStart - 30, str3);
	

	QFont font = painter->font();
	int backup = font.pointSize();
	font.setPointSize(font.pointSize() * 1.1);
	painter->setFont(font);

	painter->drawText(xStart - 130, yStart + 2, name); 
	font.setPointSize(backup);
	painter->setFont(font);

}

bool HorizontalSlider::wasClicked(int x, int y, int* valuePtr)
{
	bool retVal = false; 
	if (x >= xStart && x <= xStart + 2 * range
		&& y>= yStart-25 && y<= yStart+25)
	{
		retVal = true; 
		value = (x - xStart) / 2;
		QString str = QString::number(value);
		*valuePtr = 100 + value; 
	}
	return retVal;

}

HorizontalSlider::~HorizontalSlider()
{
}
