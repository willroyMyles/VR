#include "modelpickerdialog.h"



ModelPickerDialog::ModelPickerDialog() : CustomDialog()
{
	addConfirmAndCancelButtons();
	addTitle("Import Model & Texture");
	configureUi();
}


ModelPickerDialog::~ModelPickerDialog()
{
}

void ModelPickerDialog::configureUi()
{
	ScenceAndTextureHolder = new QWidget(this);
	gridLayout = new QGridLayout;
	textureWidget = new QPushButton("click to choose texture");
	modelPicker = new QPushButton("...");
	modelName = new QLineEdit();
	sceneWidget = new SceneViewWidget();

	auto hbox = new QHBoxLayout;
	hbox->addWidget(modelPicker);
	hbox->addWidget(modelName);

	ScenceAndTextureHolder->setLayout(gridLayout);

	gridLayout->addWidget(sceneWidget, 0, 0);
	gridLayout->addWidget(textureWidget, 0, 1);
	gridLayout->addLayout(hbox, 0, 0,1,2);

	insertWidget(ScenceAndTextureHolder);

}
