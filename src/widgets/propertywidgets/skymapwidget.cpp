/**************************************************************************
This file is part of JahshakaVR, VR Authoring Toolkit
http://www.jahshaka.com
Copyright (c) 2016  GPLv3 Jahshaka LLC <coders@jahshaka.com>

This is free software: you may copy, redistribute
and/or modify it under the terms of the GPLv3 License

For more information see the LICENSE file
*************************************************************************/

#include "skymapwidget.h"
#include "src/core/thumbnailmanager.h"
#include "widgets/assetpickerwidget.h"
#include "misc/stylesheet.h"
#include <QPainter>
#include <QSharedPointer>
#include <QGraphicsEffect>
#include <QAction>
#include <QMenu>

SkyMapWidget::SkyMapWidget() : QWidget()
{
	layout = new QGridLayout;
	setLayout(layout);
	top = bottom = left = right = front = back = nullptr;
	setMinimumHeight(180);

	layout->setVerticalSpacing(0);
	layout->setHorizontalSpacing(0);
}

void SkyMapWidget::addTopImage(QString topImagePath)
{
	removeCubeMapImageIfPresent(top);

	top = new CubeMapButton(topImagePath, this);
	top->setStringPosition("top");
	layout->addWidget(top, 0, 1,1,1);
}

void SkyMapWidget::addBottomImage(QString bottomImagePath)
{
	removeCubeMapImageIfPresent(bottom);

	bottom = new CubeMapButton(bottomImagePath, this);
	bottom->setStringPosition("bottom");
	layout->addWidget(bottom, 3, 1,1,1);
}

void SkyMapWidget::addLeftImage(QString leftImagePath)
{
	removeCubeMapImageIfPresent(left);

	left = new CubeMapButton(leftImagePath, this);
	left->setStringPosition("left");
	layout->addWidget(left, 1, 0,1,1);
}

void SkyMapWidget::addRightImage(QString rightImagePath)
{
	removeCubeMapImageIfPresent(right);

	right = new CubeMapButton(rightImagePath, this);
	right->setStringPosition("right");
	layout->addWidget(right, 1, 2,1,1);
}

void SkyMapWidget::addFrontImage(QString frontImagePath)
{
	removeCubeMapImageIfPresent(front);

	front = new CubeMapButton(frontImagePath, this);
	front->setStringPosition("front");
	layout->addWidget(front, 1, 1,1,1);
}

void SkyMapWidget::addBackImage(QString backImagePath)
{
	removeCubeMapImageIfPresent(back);
	back = new CubeMapButton(backImagePath, this);
	back->setStringPosition("back");
	layout->addWidget(back, 1, 3,1,1);
}

void SkyMapWidget::addCubeMapImages(QString top, QString bottom, QString left, QString front, QString right, QString back)
{
	addTopImage(top);
	addBottomImage(bottom);
	addLeftImage(left);
	addFrontImage(front);
	addRightImage(right);
	addBackImage(back);
}

void SkyMapWidget::addCubeMapImages(QStringList list)
{
	if (list.length() < 6) return;
	addCubeMapImages(list[0], list[1], list[2], list[3], list[4], list[5]);
}

void SkyMapWidget::removeCubeMapImageIfPresent(CubeMapButton* btn)
{
	if (btn != nullptr) {
		layout->removeWidget(btn);
		delete btn;
	}

}



CubeMapButton::CubeMapButton(QString imagePath, SkyMapWidget* parent) : QPushButton()
{
	path = imagePath;
	this->parent = parent;
	setImage(imagePath);

	
	configureUi();
	configureConnections();
	setMinimumHeight(60);
}

void CubeMapButton::setStringPosition(QString string)
{
	positionLabel->setText(string);
}

void CubeMapButton::setPosition(CubeMapPosition pos)
{
	position = pos;
	switch (pos) {
	case CubeMapPosition::top:
		setStringPosition("top");
		break;
	case CubeMapPosition::left:
		setStringPosition("left");
		break;
	case CubeMapPosition::front:
		setStringPosition("front");
		break;
	case CubeMapPosition::right:
		setStringPosition("right");
		break;
	case CubeMapPosition::back:
		setStringPosition("back");
	case CubeMapPosition::bottom:
		setStringPosition("bottom");
		break;
	}
}

void CubeMapButton::setImage(QString path)
{
	if (path.isNull() || path.isEmpty()) {
		image = QImage();
	}
	else {
		auto thumb = ThumbnailManager::createThumbnail(path, 60, height());
		image = *thumb->thumb;
		image = image.scaled(QSize(28, 28));
	}
	this->path = path;

	emit parent->valueChanged(path, position);
}

void CubeMapButton::configureConnections()
{
	

	connect(this, &CubeMapButton::clicked, [=]() {
		//create menu

		
		auto select = new QAction("Select");
		auto clear = new QAction("Clear");

		auto rotate360 = new QAction("Roate image to 360 degrees");
		auto rotate90 = new QAction("Roate image to 90 degrees");
		auto rotate180 = new QAction("Roate image to 180 degrees");
		auto rotate270 = new QAction("Roate image to 270 degrees");

		auto flipHorizontal = new QAction("Flip Horizontally");
		auto flipVertical = new QAction("Flip Vertically");

		auto rotate = new QMenu("Rotate");
		auto flip = new QMenu("Flip");
		rotate->addActions({ rotate90, rotate180, rotate270, rotate360 });
		flip->addActions({ flipHorizontal, flipVertical });
		

		auto menu = new QMenu();
		menu->addActions({ select, clear });
		menu->addMenu(rotate);
		menu->addMenu(flip);
		menu->setStyleSheet(StyleSheet::QMenu());
		flip->setStyleSheet(StyleSheet::QMenu());
		rotate->setStyleSheet(StyleSheet::QMenu());

		connect(select, &QAction::triggered, [=]() {
			selectImage();
			});
		connect(clear, &QAction::triggered, [=]() {
			clearImage();
			});
		connect(rotate360, &QAction::triggered, [=]() {
			rotateImage(360);
			});
		connect(rotate90, &QAction::triggered, [=]() {
			rotateImage(90);
			});
		connect(rotate180, &QAction::triggered, [=]() {
			rotateImage(180);
			});
		connect(rotate270, &QAction::triggered, [=]() {
			rotateImage(270);
			});
		connect(flipHorizontal, &QAction::triggered, [=]() {
			flipImage(Qt::Orientation::Horizontal);
			});
		connect(flipVertical, &QAction::triggered, [=]() {
			flipImage(Qt::Orientation::Vertical);
			});


		menu->exec(mapToGlobal(QPoint(0,0)));

		

	});

	
}

void CubeMapButton::configureUi()
{
	
	layout = new QVBoxLayout;
	positionLabel = new QLabel();
	container = new QWidget;
	select = new QPushButton("Select");
	clear = new QPushButton("Clear");
	auto containerLayout = new QVBoxLayout;
	container->setLayout(containerLayout);

	containerLayout->addWidget(positionLabel);
	

	layout->addWidget(container);
	container->setVisible(false);
	setCursor(Qt::PointingHandCursor);
	setLayout(layout);

	container->setStyleSheet(StyleSheet::QWidgetTransparent());
	positionLabel->setStyleSheet(StyleSheet::QLabelWhite());
	select->setStyleSheet(StyleSheet::QPushButtonGreyscale());
	clear->setStyleSheet(StyleSheet::QPushButtonGreyscale());

	//add dropShadow to QLabel to stand out from backgrounds
	auto effect = new QGraphicsDropShadowEffect(positionLabel);
	effect->setBlurRadius(10);
	effect->setOffset(0);
	effect->setColor(QColor(0, 0, 0, 200));
	positionLabel->setGraphicsEffect(effect);
	positionLabel->setAlignment(Qt::AlignCenter);

	QFont f = positionLabel->font();
	f.setWeight(65);
	positionLabel->setFont(f);

	layout->setContentsMargins(0, 0, 0, 0);
	containerLayout->setContentsMargins(2, 2, 2, 2);
}

void CubeMapButton::paintEvent(QPaintEvent* event)
{
	QPushButton::paintEvent(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	if (!image.isNull()) {
		
		//painter.drawPixmap(0, 0, width(), height(), pixmap);
		painter.drawImage(QRect(0, 0, width(), height()), image);
	}
	else {
		painter.fillRect(0, 0, width(), height(), QColor(0, 0, 0, 0));
	}
	if (container->isVisible()) {
		painter.setPen(QPen(QColor(20,20,20, 40), 2));
		painter.drawRect(0, 0, width(), height());
	}
}

void CubeMapButton::enterEvent(QEvent* event)
{
	QPushButton::enterEvent(event);
	container->setVisible(true);
}

void CubeMapButton::leaveEvent(QEvent* event)
{
	QPushButton::leaveEvent(event);
	container->setVisible(false);
}

void CubeMapButton::selectImage()
{
	auto widget = new AssetPickerWidget(ModelTypes::Texture);
	connect(widget, &AssetPickerWidget::itemDoubleClicked, [=](QListWidgetItem * item) {
		setImage(item->data(Qt::UserRole).toString());
		}); 
}

void CubeMapButton::clearImage()
{
	path.clear();
	pixmap = QPixmap();
	image = QImage();
	emit parent->valueChanged(path, position);
}

void CubeMapButton::rotateImage(int degrees)
{
	switch (degrees) {
		case static_cast<int>(Rotation::Ninety) :
			rotation = Rotation::Ninety;
			break;
		case static_cast<int>(Rotation::OneEighty):
			rotation = Rotation::OneEighty;
			break;
		case static_cast<int>(Rotation::TwoSeventy) :
			rotation = Rotation::TwoSeventy;
			break;
		case static_cast<int>(Rotation::ThreeSixty) :
			rotation = Rotation::ThreeSixty;
			break;
		default:
			rotation = Rotation::Zero;
			break;
	}
}

void CubeMapButton::flipImage(Qt::Orientation orientation)
{
}