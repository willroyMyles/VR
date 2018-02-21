/**************************************************************************
This file is part of JahshakaVR, VR Authoring Toolkit
http://www.jahshaka.com
Copyright (c) 2016  GPLv3 Jahshaka LLC <coders@jahshaka.com>

This is free software: you may copy, redistribute
and/or modify it under the terms of the GPLv3 License

For more information see the LICENSE file
*************************************************************************/
#include "worldsettings.h"
#include "ui_worldsettings.h"
#include "../../core/settingsmanager.h"
#include "../../constants.h"
#include "../../uimanager.h"
#include "../../globals.h"
#include "../../widgets/sceneviewwidget.h"
#include "../../core/database/database.h"
#include <QFileDialog>
#include <QStandardPaths>

WorldSettings::WorldSettings(Database *handle, SettingsManager* settings) :
    QWidget(nullptr),
    ui(new Ui::WorldSettings)
{
    ui->setupUi(this);
	db = handle;

    this->settings = settings;

	ui->author->setText(db->getAuthorName());

    connect(ui->browseProject, SIGNAL(pressed()), SLOT(changeDefaultDirectory()));
    connect(ui->outlineWidth, SIGNAL(valueChanged(double)), SLOT(outlineWidthChanged(double)));
    connect(ui->outlineColor, SIGNAL(onColorChanged(QColor)), SLOT(outlineColorChanged(QColor)));
    connect(ui->projectDefault, SIGNAL(textChanged(QString)), SLOT(projectDirectoryChanged(QString)));
    connect(ui->showFPS, SIGNAL(toggled(bool)), SLOT(showFpsChanged(bool)));
	connect(ui->autoSave, SIGNAL(toggled(bool)), SLOT(enableAutoSave(bool)));
	connect(ui->openInPlayer, SIGNAL(toggled(bool)), SLOT(enableOpenInPlayer(bool)));

    setupDirectoryDefaults();
    setupOutline();

	showFps = settings->getValue("show_fps", false).toBool();
	ui->showFPS->setChecked(showFps);

	autoSave = settings->getValue("auto_save", true).toBool();
	ui->autoSave->setChecked(autoSave);

	openInPlayer = settings->getValue("open_in_player", false).toBool();
}

void WorldSettings::setupOutline()
{
    outlineWidth = settings->getValue("outline_width", 6).toInt();
    outlineColor = settings->getValue("outline_color", "#3498db").toString();

    ui->outlineWidth->setValue(outlineWidth);
    ui->outlineColor->setColor(outlineColor);
}

void WorldSettings::changeDefaultDirectory()
{
    QFileDialog projectDir;
    defaultProjectDirectory = projectDir.getExistingDirectory(nullptr, "Select project dir", defaultProjectDirectory);
    if (!defaultProjectDirectory.isNull())
        ui->projectDefault->setText(defaultProjectDirectory);
}

void WorldSettings::outlineWidthChanged(double width)
{
    settings->setValue("outline_width", (int) width);
    outlineWidth = width;
}

void WorldSettings::outlineColorChanged(QColor color)
{
    settings->setValue("outline_color", color.name());
    outlineColor = color;
}

void WorldSettings::showFpsChanged(bool show)
{
    showFps = show;
    if (UiManager::sceneViewWidget) UiManager::sceneViewWidget->setShowFps(show);
}

void WorldSettings::enableAutoSave(bool state)
{
	settings->setValue("auto_save", autoSave = state);
}

void WorldSettings::enableOpenInPlayer(bool state)
{
	settings->setValue("open_in_player", openInPlayer = state);
}

void WorldSettings::projectDirectoryChanged(QString path)
{
    settings->setValue("default_directory", path);
    defaultProjectDirectory = path;
}

void WorldSettings::saveSettings()
{
	if (!ui->author->text().isEmpty()) {
		db->updateAuthorInfo(ui->author->text());
	}
}

WorldSettings::~WorldSettings()
{
    delete ui;
}

void WorldSettings::setupDirectoryDefaults()
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                + Constants::PROJECT_FOLDER;
    defaultProjectDirectory = settings->getValue("default_directory", path).toString();

    ui->projectDefault->setText(defaultProjectDirectory);
}
