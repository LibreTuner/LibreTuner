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

#include "interfacemanager.h"
#include "libretuner.h"

InterfaceManager &InterfaceManager::get() {
  static InterfaceManager i;
  return i;
}

InterfaceManager::InterfaceManager() : signal_(SignalType::create()) {}

gsl::span<const InterfaceSettingsPtr> InterfaceManager::settings() {
  return settings_;
}

void InterfaceManager::load() {
  QFile file(QString::fromStdString(path()));
  if (!file.open(QFile::ReadOnly)) {
    // Nothing to load
    return;
  }

  QXmlStreamReader xml(&file);
  if (xml.readNextStartElement()) {
    if (xml.name() != "interfaces") {
      xml.raiseError(QObject::tr("Unexpected element"));
    } else {
      while (xml.readNextStartElement()) {
        if (xml.name() == "interface") {
          if (xml.attributes().hasAttribute("type")) {
            InterfaceType type = InterfaceSettings::type(
                xml.attributes().value("type").toString().toStdString());
            if (type == InterfaceType::Invalid) {
              xml.raiseError("Invalid type");
              break;
            }
            InterfaceSettingsPtr iface = InterfaceSettings::create(type);
            if (!iface) {
              xml.raiseError("Unsupported interface");
              break;
            }
            iface->load(xml);
            if (xml.hasError()) {
              break;
            }
            settings_.emplace_back(std::move(iface));
          }
        } else {
          xml.raiseError("Unexpected element");
        }
      }
    }
  }
  file.close();
  if (xml.hasError()) {
    throw std::runtime_error(QObject::tr("%1\nLine %2, column %3")
                                 .arg(xml.errorString())
                                 .arg(xml.lineNumber())
                                 .arg(xml.columnNumber())
                                 .toStdString());
  }
}

void InterfaceManager::save() {
  QFile file(QString::fromStdString(path()));
  if (!file.open(QFile::WriteOnly)) {
    throw std::runtime_error("Could not open file for writing: " +
                             file.errorString().toStdString());
  }

  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);
  xml.setAutoFormattingIndent(-1); // tabs > spaces

  xml.writeStartDocument();
  xml.writeDTD("<!DOCTYPE interfaces>");
  xml.writeStartElement("interfaces");

  for (const InterfaceSettingsPtr &iface : settings_) {
    iface->save(xml);
  }

  xml.writeEndElement();
  xml.writeEndDocument();
  file.close();
}

void InterfaceManager::add(const InterfaceSettingsPtr &iface) {
  assert(iface);
  settings_.emplace_back(iface);
  save();
  signal_->call(settings_);
}

void InterfaceManager::remove(const InterfaceSettingsPtr &iface) {
  assert(iface);
  settings_.erase(std::remove(settings_.begin(), settings_.end(), iface),
                  settings_.end());
  save();
  signal_->call(settings_);
}

std::string InterfaceManager::path() {
  return (LibreTuner::get()->home() + "/interfaces.xml").toStdString();
}
