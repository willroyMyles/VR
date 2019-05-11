#pragma once

#include "customdialog.h"
#include "../widgets/sceneviewwidget.h"
#include "../widgets/assetviewer.h"
#include "src/core/database/database.h"

#include <QSplitter>

class ModelPickerDialog : public CustomDialog
{
public:
	ModelPickerDialog(Database *d);
	~ModelPickerDialog();

	void configureUi();

private:
	AssetViewer *assetViewer;
	QPushButton* textureWidget;
	QPushButton* modelPicker;
	QLineEdit* modelName;
	QWidget* ScenceAndTextureHolder;
	QGridLayout* gridLayout;
	Database* db;

	void setUpConnections();
	void importJahModel(const QString& fileName);
};

