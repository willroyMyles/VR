#pragma once

#include "customdialog.h"
#include "src/core/database/database.h"

#include <QSplitter>
#include <QScrollArea>
#include <QStackedWidget>
#include <QCheckBox>

class AssetViewer;
class ModelPickerDialog : public CustomDialog
{
	Q_OBJECT
public:
	ModelPickerDialog(Database *d);
	~ModelPickerDialog();

	void configureUi();

private:
	AssetViewer *assetViewer;
	QPushButton* textureWidget;
	QPushButton* modelPicker;
	QPushButton* texturePicker;
	QLineEdit* modelName;
	QWidget* scenceAndTextureHolder;
	QGridLayout* grid;
	Database* db;
	QString textureFileName = "";

	QWidget* modelHolder;
	QWidget* textureHolder;
	QStackedWidget* stackWidget;
	QCheckBox* modelCheck;
	QCheckBox* textureCheck;

	bool modelPicked = false;
	bool texturePicked = false;
	

	void setUpConnections();
	void importJahModel(const QString& fileName);

signals:
	void textureChanged(QString texture);
	void modelChanged();
};

