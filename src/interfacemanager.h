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

#include "datalink.h"
#include "interface.h"
#include "util/signal.h"

#include <functional>
#include <gsl/span>

class InterfaceManager {
public:
  using ChangeCall = std::function<void(gsl::span<const InterfaceSettingsPtr>)>;
  using SignalType = Signal<ChangeCall>;
  using ConnType = SignalType::ConnectionType;

  static InterfaceManager &get();

  std::shared_ptr<ConnType> connect(ChangeCall &&call) {
    return signal_->connect(std::move(call));
  }

  gsl::span<const InterfaceSettingsPtr> settings();

  void add(const InterfaceSettingsPtr &iface);
  void remove(const InterfaceSettingsPtr &iface);

  /* Returns the default interface, if one exists */
  InterfaceSettingsPtr defaultInterface();

  void load();
  void save();

  std::string path();

private:
  std::vector<InterfaceSettingsPtr> settings_;
  std::shared_ptr<SignalType> signal_;

  InterfaceSettingsPtr default_;

  void resetDefault();

  InterfaceManager();
};

#endif // LIBRETUNER_INTERFACEMANAGER_H
