#include "Canvas.h"
#include <QPainter>
#include <QString>
#include <QFile>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "Regression.h"
#include <time.h>

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);

	this->setFocusPolicy(Qt::StrongFocus);

	regression = new Regression("../models/rectangles_41/deploy.prototxt", "../models/rectangles_41/train.caffemodel");
	predicted = false;
}

void Canvas::clear() {
	image.fill(qRgba(255, 255, 255, 0));
	predicted = false;
	update();
}

void Canvas::resizeImage(const QSize &newSize) {
	QImage newImage(newSize, QImage::Format_RGB888);
	newImage.fill(qRgba(255, 255, 255, 0));
	QPainter painter(&newImage);
	painter.drawImage(QPoint(0, 0), image);
	image = newImage;
}

void Canvas::drawLineTo(const QPoint &endPoint) {
	QPoint pt1((float)lastPoint.x() - 0.5, (float)lastPoint.y() - 0.5);
	QPoint pt2((float)endPoint.x() - 0.5, (float)endPoint.y() - 0.5);

	QPainter painter(&image);
	painter.setPen(QPen(QColor(0, 0, 0), 1));
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.drawLine(pt1, pt2);

	lastPoint = endPoint;
}

void Canvas::predict() {
	time_t start = clock();

	cv::Mat mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
	cv::Mat grayMat;
	cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

	// shrink the image while keeping the black lines as much as possible
	cv::resize(grayMat, grayMat, cv::Size(256, 256));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);
	cv::resize(grayMat, grayMat, cv::Size(128, 128));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);

	params = regression->Predict(grayMat);
	predicted = true;
	
	time_t end = clock();
	std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;
	
	update();
}

void Canvas::paintEvent(QPaintEvent * /* event */) {
	QPainter painter(this);
	painter.drawImage(QPoint(0, 0), image);

	// show the result
	if (predicted) {
		int x = (params[0] * 108 + 10) * image.width() / 128;
		int y = (params[1] * 108 + 10) * image.height() / 128;
		int w = (params[2] * 108) * image.width() / 128;
		int h = (params[3] * 108) * image.height() / 128;
		painter.setPen(QPen(QColor(255, 0, 0), 2));
		painter.drawRect(x, y, w, h);
	}
}

void Canvas::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		lastPoint = event->pos();
	}
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
	drawLineTo(event->pos());
	update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
	predict();
}

void Canvas::resizeEvent(QResizeEvent *event) {
	resizeImage(event->size());
	QWidget::resizeEvent(event);
}

void Canvas::keyPressEvent(QKeyEvent* event) {
}

void Canvas::keyReleaseEvent(QKeyEvent* event) {
}
