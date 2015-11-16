#pragma once

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>
#include <iostream>

class Regression;

class Canvas : public QWidget {
	Q_OBJECT

private:
	QImage image;
	QPoint lastPoint;
	Regression* regression;
	std::vector<float> params;
	bool predicted;

public:
	Canvas(QWidget* parent = 0);

	void clear();
	void resizeImage(const QSize &newSize);
	void drawLineTo(const QPoint &endPoint);
	void predict();

protected:
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent* keyEvent);
	void keyReleaseEvent(QKeyEvent* keyEvent);
};

