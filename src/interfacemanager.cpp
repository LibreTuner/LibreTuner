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

std::vector<InterfaceSettingsPtr> &InterfaceManager::settings() {
    return settings_;
}

std::vector<InterfaceSettingsPtr> &InterfaceManager::autosettings() {
    return autosettings_;
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
                        InterfaceType type =
                            InterfaceSettings::type(xml.attributes()
                                                        .value("type")
                                                        .toString()
                                                        .toStdString());
                        if (type == InterfaceType::Invalid) {
                            xml.raiseError("Invalid type");
                            break;
                        }
                        InterfaceSettingsPtr iface =
                            InterfaceSettings::create(type);
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
    resetDefault();
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
    if (!default_) {
        resetDefault();
    }
    save();
    signal_->call();
    
    // TODO: Emit change signal
}

void InterfaceManager::remove(const InterfaceSettingsPtr &iface) {
    assert(iface);
    settings_.erase(std::remove(settings_.begin(), settings_.end(), iface),
                    settings_.end());
    if (default_ == iface) {
        resetDefault();
    }
    save();
    signal_->call();
    
    // TODO: Emit change signal
}

void InterfaceManager::addAuto(const InterfaceSettingsPtr &iface) {
    autosettings_.emplace_back(iface);
    if (!default_) {
        resetDefault();
    }
    signal_->call();
}

void InterfaceManager::clearAuto() {
    autosettings_.clear();
    resetDefault();
    signal_->call();
}

void InterfaceManager::resetDefault() {
    if (settings_.empty()) {
        if (!autosettings_.empty()) {
            default_ = autosettings_.front();
            return;
        }
        default_ = nullptr;
        return;
    }
    default_ = settings_.front();
}

std::string InterfaceManager::path() {
    return (LibreTuner::get()->home() + "/interfaces.xml").toStdString();
}

InterfaceSettingsPtr InterfaceManager::defaultInterface() { return default_; }

void InterfaceList::addManual(const InterfaceSettingsPtr &iface) {
    manualSettings_.emplace_back(iface);
}

void InterfaceList::removeManual(const InterfaceSettingsPtr &iface) {
    manualSettings_.erase(
        std::remove(manualSettings_.begin(), manualSettings_.end(), iface),
        manualSettings_.end());
}

Q_DECLARE_METATYPE(InterfaceSettings*)

QVariant InterfaceManager::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::UserRole)) {
        return QVariant();
    }
    
    if (index.column() != 0) {
        return QVariant();
    }
    
    int row = index.row();
    if (row < 0) {
        return QVariant();
    }
    
    // Settings first
    if (row < settings_.size()) {
        switch (role) {
            case Qt::DisplayRole:
                return QString::fromStdString(settings_[row]->name());
            case Qt::UserRole:
                return QVariant::fromValue<InterfaceSettings*>(settings_[row].get());
        }
    }
    
    // Auto-detect settings
    row -= settings_.size();
    if (row < autosettings_.size()) {
        switch (role) {
            case Qt::DisplayRole:
                return QString::fromStdString(autosettings_[row]->name());
            case Qt::UserRole:
                return QVariant::fromValue<InterfaceSettings*>(autosettings_[row].get());
        }
    }
    
    return QVariant();
}



int InterfaceManager::rowCount(const QModelIndex& parent) const
{
    return settings_.size() + autosettings_.size();
}

