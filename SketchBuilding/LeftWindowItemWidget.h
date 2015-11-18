#pragma once

#include <QLabel>
#include <QImage>
#include <QString>
#include <QMouseEvent>

class MainWindow;

class LeftWindowItemWidget : public QLabel {
	Q_OBJECT

public:
	static const int WIDGET_WIDTH = 200;
	static const int WIDGET_HEIGHT = 200;
	static const int IMAGE_WIDTH = 180;
	static const int IMAGE_HEIGHT = 180;

public:
	MainWindow* mainWin;
	int option_index;

signals:
	void clicked(int);

public:
	LeftWindowItemWidget(QWidget *parent, const QString& text, const QImage& image, int option_index);

protected:
	void mousePressEvent(QMouseEvent* e);

public slots:
	void onClicked(int option_index);
};

