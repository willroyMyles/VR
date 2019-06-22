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

#include "src/core/assethelper.h"
#include "globals.h"
#include "uimanager.h"
#include "src/core/thumbnailmanager.h"
#include "src/io/assetmanager.h"
#include "src/widgets/assetwidget.h"
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

	modelPicker = new QPushButton("Choose model");
	texturePicker = new QPushButton("Choose texture");

	modelHolder = new QWidget;

	modelPath = new QLineEdit;
	texturePath = new QLineEdit;

	auto grid = new QGridLayout;
	grid->setHorizontalSpacing(0);
	grid->addWidget(modelPath, 0, 0);
	grid->addWidget(texturePath, 1, 0);
	grid->addWidget(modelPicker, 0, 1);
	grid->addWidget(texturePicker, 1, 1);
	modelHolder->setLayout(grid);


	holder->setMinimumWidth(450);
	insertWidget(modelHolder);
	modelPicker->setStyleSheet(StyleSheet::QPushButtonGreyscale());
	texturePicker->setStyleSheet(StyleSheet::QPushButtonGreyscale());
	modelPath->setStyleSheet(StyleSheet::QLineEdit());
	texturePath->setStyleSheet(StyleSheet::QLineEdit());

	setUpConnections();
	qDebug() << modelHolder->geometry();
	
}

void ModelPickerDialog::importModelAndTextureAndCreateDependency()
{
	QString modelGuid;
	QString textureGuid;
	if (modelPicked) modelGuid = importJahModel(modelPath->text());
	if (texturePicked) textureGuid = importTexture(texturePath->text());
	if (modelPicked && texturePicked) {
		//createDependency
		db->createDependency(static_cast<int>(ModelTypes::Object), static_cast<int>(ModelTypes::Texture), modelGuid, textureGuid);
	}
}

void ModelPickerDialog::setUpConnections()
{
	connect(modelPicker, &QPushButton::clicked, [=]() {
		auto p = QFileDialog::getOpenFileName(this, "Load Model");
		if (p.isEmpty()) return;
		else {
			modelPicked = true;
			modelPath->setText(p);
			//importJahModel(p);
		}
		});
	connect(texturePicker, &QPushButton::clicked, [=]() {
		auto p = QFileDialog::getOpenFileName(this, "Load Texture");
		if (p.isEmpty()) return;
		else {
			texturePath->setText(p);
			emit textureChanged(p);
			texturePicked = true;
			//importTexture(p);
			}
		});
}

QString ModelPickerDialog::importJahModel(const QString& fileName)
{
	QFileInfo entryInfo(fileName);
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
		}

		if (!f.open(QFile::ReadOnly | QFile::Text)) return QString::null;
		QTextStream in(&f);
		const QString jafString = in.readLine();
		f.close();

		// Copy assets over to project folder
		// If the file already exists, increment the filename and do the same when inserting the db entry
		// get all the files and directories in the project working directory
		QString assetsDir = QDir(temporaryDir.path()).filePath("assets");
		QDirIterator projectDirIterator(assetsDir, QDir::NoDotAndDotDot | QDir::Files);

		QStringList fileNames;
		while (projectDirIterator.hasNext()) fileNames << projectDirIterator.next();

		// Create a pair that holds the original name and the new name (if any)
		QVector<QPair<QString, QString>> files;	/* original x new */
		QStringList fullFileList;

		for (const auto& image : fileNames) {
			if (Constants::IMAGE_EXTS.contains(QFileInfo(image).suffix().toLower())) {
				fullFileList.append(image);
			}
		}

		for (const auto& material : fileNames) {
			if (QFileInfo(material).suffix() == Constants::MATERIAL_EXT) {
				fullFileList.append(material);
			}
		}

		for (const auto& file : fileNames) {
			if (Constants::WHITELIST.contains(QFileInfo(file).suffix().toLower())) {
				fullFileList.append(file);
			}
		}

		for (const auto& shader : fileNames) {
			if (QFileInfo(shader).suffix() == Constants::SHADER_EXT) {
				fullFileList.append(shader);
			}
		}

		for (const auto& mesh : fileNames) {
			if (Constants::MODEL_EXTS.contains(QFileInfo(mesh).suffix().toLower())) {
				fullFileList.append(mesh);
			}
		}

		QMap<QString, QString> guidCompareMap; /* old x new */

		QString placeHolderGuid = GUIDManager::generateGUID();

		for (const auto& file : fullFileList) {
			QFileInfo fileInfo(file);
			ModelTypes jafType = AssetHelper::getAssetTypeFromExtension(fileInfo.suffix().toLower());

			QString pathToCopyTo = Globals::project->getProjectFolder();
			QString fileToCopyTo = IrisUtils::join(pathToCopyTo, fileInfo.fileName());

			int increment = 1;
			QFileInfo checkFile(fileToCopyTo);

			// If we encounter the same file, make a duplicate...
			QString newFileName = fileInfo.fileName();

			while (checkFile.exists()) {
				QString newName = QString(fileInfo.baseName() + " %1").arg(QString::number(increment++));
				checkFile = QFileInfo(IrisUtils::buildFileName(
					IrisUtils::join(pathToCopyTo, newName), fileInfo.suffix())
				);
				newFileName = checkFile.fileName();
			}

			files.push_back(QPair<QString, QString>(file, QDir(pathToCopyTo).filePath(newFileName)));
			bool copyFile = QFile::copy(file, QDir(pathToCopyTo).filePath(newFileName));

		}

		QMap<QString, QString> newNames;	/* original x new */
		for (const auto& file : files) {
			newNames.insert(
				QFileInfo(file.first).fileName(),
				QFileInfo(file.second).fileName()
			);
		}


		// We can discern most types from their extension, we don't store material files so we use the manifest
		ModelTypes jafType;
		if (jafString == "material") {
			jafType = ModelTypes::Material;
		}
		else if (jafString == "texture") {
			jafType = ModelTypes::Texture;
		}
		else if (jafString == "object") {
			jafType = ModelTypes::Object;
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
		else {
			// Default to files since we know what archives can contain
			jafType = ModelTypes::File;
		}

		QVector<AssetRecord> oldAssetRecords;

		return  db->importAsset(
			jafType,
			QDir(temporaryDir.path()).filePath("asset.db"),
			newNames,
			guidCompareMap,
			oldAssetRecords,
			AssetViewFilter::Editor,
			Globals::project->getProjectGuid()
			
		);

		// The multiple for loops are intentional, don't try to optimize this, each asset type must be updated
		//for (auto& asset : AssetManager::getAssets()) {
		//	if (asset->type == ModelTypes::File) {
		//		for (const auto& record : oldAssetRecords) {
		//			if (record.name == asset->fileName) {
		//				asset->assetGuid = record.guid;
		//			}
		//		}
		//	}
		//}

		//if (jafType == ModelTypes::Texture) {
		//	for (auto& asset : AssetManager::getAssets()) {
		//		if (asset->assetGuid == placeHolderGuid && asset->type == ModelTypes::Texture) {
		//			asset->assetGuid = guidReturned;
		//		}
		//	}
		//}

		//if (jafType == ModelTypes::Shader) {
		//	QJsonDocument matDoc = QJsonDocument::fromBinaryData(db->fetchAssetData(guidReturned));
		//	QJsonObject shaderDefinition = matDoc.object();

		//	auto assetShader = new AssetShader;
		//	assetShader->assetGuid = guidReturned;
		//	assetShader->fileName = db->fetchAsset(guidReturned).name;
		//	assetShader->setValue(QVariant::fromValue(shaderDefinition));
		//	AssetManager::addAsset(assetShader);
		//}
		//else {
		//	for (const auto& asset : oldAssetRecords) {
		//		if (asset.type == static_cast<int>(ModelTypes::Shader)) {
		//			QJsonDocument matDoc = QJsonDocument::fromBinaryData(asset.asset);
		//			QJsonObject shaderDefinition = matDoc.object();

		//			auto assetShader = new AssetShader;
		//			assetShader->assetGuid = asset.guid;
		//			assetShader->fileName = asset.name;
		//			assetShader->setValue(QVariant::fromValue(shaderDefinition));
		//			AssetManager::addAsset(assetShader);
		//		}
		//	}
		//}

		//if (jafType == ModelTypes::Sky) {
		//	// No need to do anything really
		//}

		//if (jafType == ModelTypes::Material) {
		//	QJsonDocument matDoc = QJsonDocument::fromBinaryData(db->fetchAssetData(guidReturned));
		//	QJsonObject matObject = matDoc.object();

		//	MaterialReader reader;
		//	auto material = reader.parseMaterial(matObject, db);
		//	/*
		//	iris::CustomMaterialPtr material = iris::CustomMaterialPtr::create();

		//	QFileInfo shaderFile;

		//	QMapIterator<QString, QString> it(Constants::Reserved::BuiltinShaders);
		//	while (it.hasNext()) {
		//		it.next();
		//		if (it.key() == matObject["guid"].toString()) {
		//			shaderFile = QFileInfo(IrisUtils::getAbsoluteAssetPath(it.value()));
		//			break;
		//		}
		//	}

		//	if (shaderFile.exists()) {
		//		material->generate(shaderFile.absoluteFilePath());
		//	}
		//	else {
		//		for (auto asset : AssetManager::getAssets()) {
		//			if (asset->type == ModelTypes::Shader) {
		//				if (asset->assetGuid == matObject["guid"].toString()) {
		//					auto def = asset->getValue().toJsonObject();
		//					auto vertexShader = def["vertex_shader"].toString();
		//					auto fragmentShader = def["fragment_shader"].toString();
		//					for (auto asset : AssetManager::getAssets()) {
		//						if (asset->type == ModelTypes::File) {
		//							if (vertexShader == asset->assetGuid) vertexShader = asset->path;
		//							if (fragmentShader == asset->assetGuid) fragmentShader = asset->path;
		//						}
		//					}
		//					def["vertex_shader"] = vertexShader;
		//					def["fragment_shader"] = fragmentShader;
		//					material->generate(def);
		//				}
		//			}
		//		}
		//	}

		//	for (const auto &prop : material->properties) {
		//		if (prop->type == iris::PropertyType::Color) {
		//			QColor col;
		//			col.setNamedColor(matObject.value(prop->name).toString());
		//			material->setValue(prop->name, col);
		//		}
		//		else if (prop->type == iris::PropertyType::Texture) {
		//			QString materialName = db->fetchAsset(matObject.value(prop->name).toString()).name;
		//			QString textureStr = IrisUtils::join(Globals::project->getProjectFolder(), materialName);
		//			material->setValue(prop->name, !materialName.isEmpty() ? textureStr : QString());
		//		}
		//		else {
		//			material->setValue(prop->name, QVariant::fromValue(matObject.value(prop->name)));
		//		}
		//	}
		//	*/
		//	auto assetMat = new AssetMaterial;
		//	assetMat->assetGuid = guidReturned;
		//	assetMat->setValue(QVariant::fromValue(material));
		//	AssetManager::addAsset(assetMat);
		//}

		//if (jafType == ModelTypes::ParticleSystem) {
		//	QJsonDocument particleDoc = QJsonDocument::fromBinaryData(db->fetchAssetData(guidReturned));
		//	QJsonObject particleObject = particleDoc.object();

		//	auto assetPS = new AssetParticleSystem;
		//	assetPS->assetGuid = guidReturned;
		//	assetPS->setValue(QVariant::fromValue(particleObject));
		//	AssetManager::addAsset(assetPS);
		//}

		//if (jafType == ModelTypes::Object) {
		//	for (auto& asset : AssetManager::getAssets()) {
		//		if (asset->assetGuid == placeHolderGuid && asset->type == ModelTypes::Object) {
		//			asset->assetGuid = guidReturned;
		//			auto node = asset->getValue().value<iris::SceneNodePtr>();

		//			auto materialObj = QJsonDocument::fromBinaryData(db->fetchAssetData(asset->assetGuid));
		//			QJsonObject matObject = materialObj.object();

		//			//node.staticCast<iris::MeshNode>()->setMaterial(material);
		//			AssetHelper::updateNodeMaterial(node, materialObj.object());
		//		}
		//	}
		//}
	}

}

QString ModelPickerDialog::importTexture(const QString& filename)
{
	QFileInfo entryInfo(filename);
	QList<directory_tuple> imagesInUse;


	if (entryInfo.isDir()) {

	}
	else {

		ModelTypes type;
		QPixmap thumbnail = QPixmap(":/icons/empty_object.png");

		auto asset = new AssetVariant;
		asset->type = AssetHelper::getAssetTypeFromExtension(entryInfo.suffix().toLower());
		asset->fileName = entryInfo.fileName();
		asset->path = filename;
		asset->thumbnail = thumbnail;

		if (asset->type != ModelTypes::Undefined) {
			QString pathToCopyTo = Globals::project->getProjectFolder();
			QString fileToCopyTo = IrisUtils::join(pathToCopyTo, asset->fileName);

			int increment = 1;
			QFileInfo checkFile(fileToCopyTo);

			// If we encounter the same file, make a duplicate...
			// Maybe ask the user to replace sometime later on (iKlsR)
			while (checkFile.exists()) {
				// Repeatedly test if a file exists by incrementally adding a numeral to the base name
				QString newName = QString(entryInfo.baseName() + " %1").arg(QString::number(increment++));
				checkFile = QFileInfo(
					IrisUtils::buildFileName(IrisUtils::join(pathToCopyTo, newName), entryInfo.suffix())
				);
				asset->fileName = checkFile.fileName();
				fileToCopyTo = checkFile.absoluteFilePath();
			}

			// Accumulate a list of all the images imported so we can use this to update references
			// If they are used in assets that depend on them such as Materials and Objects
			{
				if (asset->type == ModelTypes::Texture) {
					auto thumb = ThumbnailManager::createThumbnail(entryInfo.absoluteFilePath(), 72, 72);
					thumbnail = QPixmap::fromImage(*thumb->thumb);
				}
			}

			// Copy only models, textures and whitelisted files
			bool copyFile = QFile::copy(filename, fileToCopyTo);
			auto guid = GUIDManager::generateGUID();
			const QString assetGuid = db->createAssetEntry(
				guid,
				filename,
				static_cast<int>(asset->type),
				Globals::project->getProjectGuid(),
				QString(),
				QString(),
				AssetHelper::makeBlobFromPixmap(thumbnail),
				NULL,
				NULL,
				NULL,
				AssetViewFilter::Editor);

			return guid;

		
		}
	}
} 
