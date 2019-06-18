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
	QLineEdit* modelPath;
	QLineEdit* texturePath;


private:
	
	QPushButton* modelPicker;
	QPushButton* texturePicker;
	QGridLayout* grid;
	Database* db;

	QWidget* modelHolder;
	QWidget* textureHolder;

	bool modelPicked = false;
	bool texturePicked = false;
	

	void setUpConnections();
	void importJahModel(const QString& fileName);

signals:
	void textureChanged(QString texture);
	void modelChanged();
};

