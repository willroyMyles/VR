#include "modelpickerdialog.h"
#include "src/core/guidmanager.h"
#include "src/constants.h"
#include "zip.h"
#include "../widgets/assetviewer.h"
#include "src/misc/stylesheet.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTemporaryDir>
#include <QMessageBox>
#include <QDirIterator>
#include <QDebug>
#include <QCheckBox>
ModelPickerDialog::ModelPickerDialog(Database *d) : CustomDialog()
{
	db = d;
	addConfirmAndCancelButtons();
	addTitle("Import Model & Texture");
	configureUi();
}


ModelPickerDialog::~ModelPickerDialog()
{
}

void ModelPickerDialog::configureUi()
{
	scenceAndTextureHolder = new QWidget;
	textureWidget = new QPushButton;
	modelPicker = new QPushButton("Choose model");
	texturePicker = new QPushButton("Choose texture");
	modelName = new QLineEdit();
	assetViewer = new AssetViewer();
	modelHolder = new QWidget;
	textureHolder = new QWidget;
	stackWidget = new QStackedWidget;
	modelCheck = new QCheckBox(this);
	textureCheck = new QCheckBox(this);

	modelCheck->setTristate(false);
	textureCheck->setTristate(false);

	auto vboxFinal = new QVBoxLayout;
	auto vbox1 = new QVBoxLayout;
	auto vbox2 = new QVBoxLayout;

	vbox1->addWidget(assetViewer);
	vbox1->addWidget(modelPicker);
	vbox2->addWidget(textureWidget);
	vbox2->addWidget(texturePicker);

	modelHolder->setLayout(vbox1);
	textureHolder->setLayout(vbox2);
	scenceAndTextureHolder->setLayout(vboxFinal);

	auto checkWidget = new QWidget;
	auto checkLayout = new QHBoxLayout;
	checkWidget->setLayout(checkLayout);

	checkLayout->addStretch();
	checkLayout->addWidget(modelCheck);
	checkLayout->addWidget(textureCheck);
	checkLayout->addStretch();
	checkLayout->setContentsMargins(0, 0, 0, 0);
	checkLayout->setSpacing(1);

	stackWidget->addWidget(modelHolder);
	stackWidget->addWidget(textureHolder);

	vboxFinal->addWidget(stackWidget);
	vboxFinal->addWidget(checkWidget);


	assetViewer->setFixedHeight(150);
	textureWidget->setFixedHeight(150); 
	holder->setFixedHeight(340);
	insertWidget(scenceAndTextureHolder);
	modelPicker->setStyleSheet(StyleSheet::QPushButtonGreyscale());
	texturePicker->setStyleSheet(StyleSheet::QPushButtonGreyscale());
	modelCheck->setStyleSheet(StyleSheet::QCheckBoxBlue());
	textureCheck->setStyleSheet(StyleSheet::QCheckBoxBlue());

	setUpConnections();
	qDebug() << modelHolder->geometry();
	
}

void ModelPickerDialog::setUpConnections()
{
	connect(modelPicker, &QPushButton::clicked, [=]() {
		auto p = QFileDialog::getOpenFileName(this, "Load Model");
		if (p.isEmpty()) return;
		else importJahModel(p);
		});
	connect(texturePicker, &QPushButton::clicked, [=]() {
		auto p = QFileDialog::getOpenFileName(this, "Load Texture");
		if (p.isEmpty()) return;
		else {
			textureWidget->setIcon(QIcon(p));
			textureFileName = p;
			textureWidget->setIconSize({ textureWidget->width(), textureWidget->height() });
			emit textureChanged(p);
			texturePicked = true;
			textureCheck->setChecked(texturePicked);
			}
		});
	connect(modelCheck, &QCheckBox::clicked, [=]() {
		modelCheck->setChecked(modelPicked);
		stackWidget->setCurrentIndex(0);
	});
	connect(textureCheck, &QCheckBox::clicked, [=]() {
		textureCheck->setChecked(texturePicked);
		stackWidget->setCurrentIndex(1);
	});
}

void ModelPickerDialog::importJahModel(const QString& fileName)
{
	QFileInfo entryInfo(fileName);

	auto assetPath = IrisUtils::join(
		QStandardPaths::writableLocation(QStandardPaths::DataLocation),
		"AssetStore"
	);

	// create a temporary directory and extract our project into it
	// we need a sure way to get the project name, so we have to extract it first and check the blob
	QTemporaryDir temporaryDir;
	if (temporaryDir.isValid()) {
		zip_extract(entryInfo.absoluteFilePath().toStdString().c_str(),
			temporaryDir.path().toStdString().c_str(),
			Q_NULLPTR, Q_NULLPTR
		);

		QFile f(QDir(temporaryDir.path()).filePath(".manifest"));

		if (!f.exists()) {
			QMessageBox::warning(
				this,
				"Incompatible Asset format",
				"This asset was made with a deprecated version of Jahshaka\n"
				"You can extract the contents manually and try importing as regular assets.",
				QMessageBox::Ok
			);

			return;
		}

		if (!f.open(QFile::ReadOnly | QFile::Text)) return;
		QTextStream in(&f);
		const QString jafString = in.readLine();
		f.close();

		ModelTypes jafType = ModelTypes::Undefined;

		if (jafString == "object") {
			jafType = ModelTypes::Object;
		}
		else if (jafString == "texture") {
			jafType = ModelTypes::Texture;
		}
		else if (jafString == "material") {
			jafType = ModelTypes::Material;
		}
		else if (jafString == "shader") {
			jafType = ModelTypes::Shader;
		}
		else if (jafString == "sky") {
			jafType = ModelTypes::Sky;
		}
		else if (jafString == "particle_system") {
			jafType = ModelTypes::ParticleSystem;
		}

		QVector<AssetRecord> records;

		QMap<QString, QString> guidCompareMap;
		QString guid = db->importAsset(jafType,
			QDir(temporaryDir.path()).filePath("asset.db"),
			QMap<QString, QString>(),
			guidCompareMap,
			records,
			AssetViewFilter::AssetsView);

		const QString assetFolder = QDir(assetPath).filePath(guid);
		QDir().mkpath(assetFolder);

		QString assetsDir = QDir(temporaryDir.path()).filePath("assets");
		QDirIterator projectDirIterator(assetsDir, QDir::NoDotAndDotDot | QDir::Files);

		QStringList fileNames;
		while (projectDirIterator.hasNext()) fileNames << projectDirIterator.next();

		jafType = ModelTypes::Undefined;

		QString placeHolderGuid = GUIDManager::generateGUID();

		for (const auto& file : fileNames) {
			QFileInfo fileInfo(file);
			QString fileToCopyTo = IrisUtils::join(assetFolder, fileInfo.fileName());
			bool copyFile = QFile::copy(fileInfo.absoluteFilePath(), fileToCopyTo);
		}

		/*if (jafString == "material") {
			assetViewer->setCurrentIndex(0);
			renameModelField->setText(QFileInfo(filename).baseName());
			viewer->loadJafMaterial(guid);
			addToJahLibrary(filename, guid, true);
		}

		if (jafString == "shader") {
			viewers->setCurrentIndex(0);
			renameModelField->setText(QFileInfo(filename).baseName());
			viewer->loadJafShader(guid, guidCompareMap);
			addToJahLibrary(filename, guid, true);
		}

		if (jafString == "sky") {
			viewers->setCurrentIndex(0);
			renameModelField->setText(QFileInfo(filename).baseName());
			viewer->loadJafSky(guid);
			addToJahLibrary(filename, guid, true);
		}

		if (jafString == "texture") {
			renameModelField->setText(QFileInfo(filename).baseName());

			{
				viewers->setCurrentIndex(1);
				auto assetPath = IrisUtils::join(
					QStandardPaths::writableLocation(QStandardPaths::DataLocation),
					"AssetStore",
					guid,
					db->fetchAsset(guid).name
				);

				QPixmap image(assetPath);
				assetImageCanvas->setPixmap(image.scaledToHeight(480, Qt::SmoothTransformation));
			}

			addToJahLibrary(filename, guid, true);
		}
*/
		if (jafString == "object") {
			//assetViewer->setCurrentIndex(0);
			// Open the asset
			QString path;
			// if model
			QDir dir(assetFolder);
			foreach(auto & file, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files)) {
				if (Constants::MODEL_EXTS.contains(file.suffix())) {
					path = file.absoluteFilePath();
					break;
				}
			}

			modelName->setText(QFileInfo(fileName).baseName());
			assetViewer->loadJafModel(path, guid);
			emit modelChanged();
			modelPicked = true;
			//addToJahLibrary(filename, guid, true);

			//create timer to swtch stacked widget
			QTimer* timer = new QTimer;
			connect(timer, &QTimer::timeout, [=]() {
				stackWidget->setCurrentIndex(1);
				modelCheck->setChecked(modelPicked);
				});
			timer->setSingleShot(true);
			timer->setInterval(600);
			timer->start();
			
		}
	}

	textureWidget->setVisible(true);
}
