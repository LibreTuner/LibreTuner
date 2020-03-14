/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OverboostApp.h"
#include "ui/styledwindow.h"

#include "uiutil.h"

#include <libretuner.h>

#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTextStream>

#include <fstream>
#include <iostream>
#include <memory>

#include "MainWindow.h"

static OverboostApp * _global;

namespace fs = std::filesystem;

OverboostApp::OverboostApp(int & argc, char * argv[]) : QApplication(argc, argv), rootPath_(fs::current_path())
{
    _global = this;

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(definitions);
    Q_INIT_RESOURCE(style);
    Q_INIT_RESOURCE(codes);

    setOrganizationDomain("libretuner.org");
    setApplicationName("LibreTuner");
    setLayoutDirection(Qt::LeftToRight);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    // Setup LT context
    lt::setLogCallback([](const std::string & message) { std::cout << message << std::endl; });

    // intolib rewrite

    // Setup main path
    rootPath_ = fs::current_path();

    std::cout << "Loading platforms" << std::endl;

    fs::path definitionPath = rootPath_ / "definitions";
    if (!fs::exists(definitionPath))
    {
        // Copy definitions
        std::cout << "Copying definitions to " + definitionPath.string() << std::endl;
        QDir destDir(QString::fromStdString(definitionPath.string()));
        destDir.mkpath(".");

        QDir sourceDir(":/definitions");
        QDirIterator it(sourceDir, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString path = it.next();
            QString relativePath = sourceDir.relativeFilePath(path);
            if (it.fileInfo().isDir())
            {
                destDir.mkpath(relativePath);
                continue;
            }

            if (!it.fileInfo().isFile())
                continue;

            QFile file(path);
            QString toPath(destDir.filePath(relativePath));
            std::cout << "Copying " + toPath.toStdString() << std::endl;
            if (!file.copy(toPath))
                std::cout << "Failed to copy definition: " + toPath.toStdString() << std::endl;
        }
    }

    catchCritical([&]() { platforms_.loadDirectory(definitionPath); }, tr("Error loading definitions"));

    setWindowIcon(QIcon(":/icons/libretuner_transparent.png"));

#ifdef WIN32
    {
        /*
        QFile f(":qdarkstyle/style.qss");
        if (f.exists())
        {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            setStyleSheet(ts.readAll());
        }
         */
    }
#endif

    mainWindow_ = new MainWindow;
    mainWindow_->show();
    mainWindow_->displayQuickStartDialog();
}

OverboostApp * OverboostApp::get() { return _global; }

void OverboostApp::loadDatalinks()
{
    std::cout << "Loading datalinks" << std::endl;
    /*
    links_.detect();

    try
    {
        links_.load();
    }
    catch (const std::runtime_error & err)
    {
        QMessageBox::warning(nullptr, tr("Datalink database error"),
                             tr("Failed to load datalink save data: ") +
                                 err.what());
    }
    */
}

OverboostApp::~OverboostApp() { _global = nullptr; }