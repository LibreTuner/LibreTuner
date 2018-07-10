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

#include "libretuner.h"
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#include "rommanager.h"
#include "tune.h"
#include "ui/flashwindow.h"
#include "ui/tuneeditor.h"
#include "ui/addinterfacedialog.h"
#include "tunemanager.h"
#include "timerrunloop.h"

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

static LibreTuner *_global;

LibreTuner::LibreTuner(int &argc, char *argv[]) : QApplication(argc, argv) {
  _global = this;

  Q_INIT_RESOURCE(icons);
  Q_INIT_RESOURCE(definitions);
  Q_INIT_RESOURCE(stylesheet);

  home_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

#ifdef WITH_SOCKETCAN
  SocketHandler::get()->initialize();
#endif
  TimerRunLoop::get().startWorker();

  if (!DefinitionManager::get()->load()) {
    QMessageBox msgBox;
    msgBox.setText(
        "Could not load definitions: " +
        QString::fromStdString(DefinitionManager::get()->lastError()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("DefinitionManager error");
    msgBox.exec();
  }

  if (!RomManager::get()->load()) {
    QMessageBox msgBox;
    msgBox.setText("Could not load ROM metadata from roms.xml: " +
                   RomManager::get()->lastError());
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("RomManager error");
    msgBox.exec();
  }

  if (!TuneManager::get()->load()) {
    QMessageBox msgBox;
    msgBox.setText("Could not load tune metadata from tunes.xml: " +
                   RomManager::get()->lastError());
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("TuneManager error");
    msgBox.exec();
  }

  try {
    InterfaceManager::get().load();
  } catch (const std::exception &e) {
    QMessageBox msgBox;
    msgBox.setText("Could not load interface data from interfaces.xml: " +
                   QString(e.what()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("InterfaceManager error");
    msgBox.exec();
  }

  mainWindow_ = std::unique_ptr<MainWindow>(new MainWindow);
  mainWindow_->show();

  checkHome();

  QFile file(":/stylesheet.qss");
  if (file.open(QFile::ReadOnly)) {
      setStyleSheet(file.readAll());
      file.close();
  }
}

void LibreTuner::editTune(const TunePtr &tune) {
  TuneDataPtr data = std::make_shared<TuneData>(tune);
  if (!data->valid()) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Tune data error");
    msgBox.setText(QStringLiteral("Error opening tune: ") +
                   QString::fromStdString(data->lastError()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return;
  }
  tuneEditor_.reset(new TuneEditor(data));
  tuneEditor_->show();
}

void LibreTuner::flashTune(const TunePtr &tune) {
  TuneDataPtr data = std::make_shared<TuneData>(tune);
  if (!data->valid()) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Tune data error");
    msgBox.setText(QStringLiteral("Error opening tune: ") +
                   QString::fromStdString(data->lastError()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return;
  }
  std::shared_ptr<Flashable> flash = std::make_shared<Flashable>(data);
  if (!flash->valid()) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Flash error");
    msgBox.setText(QStringLiteral("Failed to create flashable from tune: ") +
                   QString::fromStdString(flash->lastError()));
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    return;
  }
  flashWindow_ = std::make_unique<FlashWindow>(flash);
  flashWindow_->show();
}

LibreTuner *LibreTuner::get() { return _global; }

void LibreTuner::checkHome() {
  QDir home(home_);
  home.mkpath(".");
  home.mkdir("roms");
  home.mkdir("tunes");

  if (!home.exists("definitions")) {
    home.mkdir("definitions");
    // Copy definitions
    QDir dDir(":/definitions");

    for (QFileInfo &info :
         dDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort)) {
      QDir realDefDir(home.path() + "/definitions/" + info.fileName() + "/");
      realDefDir.mkpath(".");
      QDir subDir(info.filePath());
      for (QFileInfo &i : subDir.entryInfoList(
               QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort)) {
        QFile::copy(i.filePath(), realDefDir.path() + "/" + i.fileName());
      }
    }
  }
}

DataLinkPtr LibreTuner::getDataLink() {
  // Get the default interface
  InterfaceSettingsPtr def = InterfaceManager::get().defaultInterface();
  if (!def) {
    // Ask the user to create an interface
    AddInterfaceDialog dlg;
    dlg.exec();
    def = InterfaceManager::get().defaultInterface();
    if (!def) {
      // The user did not create one.
      return nullptr;
    }
  }

  try {
    return DataLink::create(def);
  } catch (const std::exception &e) {
    QMessageBox msg;
    msg.setText("Failed to create datalink from default interface: " + QString(e.what()));
    msg.setWindowTitle("DataLink Error");
    msg.setIcon(QMessageBox::Critical);
    msg.exec();
    return nullptr;
  }
}

LibreTuner::~LibreTuner() = default;
