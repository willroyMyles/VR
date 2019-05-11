#pragma once

#include "customdialog.h"
#include "../widgets/sceneviewwidget.h"

#include <QSplitter>

class ModelPickerDialog : public CustomDialog
{
public:
	ModelPickerDialog();
	~ModelPickerDialog();

	void configureUi();

private:
	SceneViewWidget *sceneWidget;
	QPushButton* textureWidget;
	QPushButton* modelPicker;
	QLineEdit* modelName;
	QWidget* ScenceAndTextureHolder;
	QGridLayout* gridLayout;

};

