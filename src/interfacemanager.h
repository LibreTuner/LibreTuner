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

#ifndef LIBRETUNER_INTERFACEMANAGER_H
#define LIBRETUNER_INTERFACEMANAGER_H

#include "datalink/datalink.h"
#include "interface.h"
#include "util/signal.h"

#include <functional>

#include <QAbstractListModel>

class InterfaceList {
public:
    // Adds an interface to the manually added list
    void addManual(const InterfaceSettingsPtr &iface);
    // Removed an interface from the manually added list
    void removeManual(const InterfaceSettingsPtr &iface);

private:
    std::vector<InterfaceSettingsPtr> manualSettings_;
    std::vector<InterfaceSettingsPtr> autoDetectSettings_;
};

class InterfaceManager : public QAbstractListModel {
public:
    using ChangeCall = std::function<void()>;
    using SignalType = Signal<ChangeCall>;
    using ConnType = SignalType::ConnectionType;

    static InterfaceManager &get();

    std::shared_ptr<ConnType> connect(ChangeCall &&call) {
        return signal_->connect(std::move(call));
    }

    std::vector<InterfaceSettingsPtr> &settings();
    std::vector<InterfaceSettingsPtr> &autosettings();

    void add(const InterfaceSettingsPtr &iface);
    void remove(const InterfaceSettingsPtr &iface);

    void addAuto(const InterfaceSettingsPtr &iface);
    void clearAuto();

    /* Returns the default interface, if one exists */
    InterfaceSettingsPtr defaultInterface();

    void load();
    void save();

    std::string path();
    
    
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

private:
    std::vector<InterfaceSettingsPtr> settings_;
    std::vector<InterfaceSettingsPtr> autosettings_;
    std::shared_ptr<SignalType> signal_;

    InterfaceSettingsPtr default_;

    void resetDefault();

    InterfaceManager();
};

#endif // LIBRETUNER_INTERFACEMANAGER_H
