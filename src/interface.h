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

#ifndef LIBRETUNER_INTERFACE_H
#define LIBRETUNER_INTERFACE_H

#include <memory>
#include <string>

#include <QXmlStreamWriter>

enum class InterfaceType {
  SocketCan,
  J2534,
  Invalid,
};

class InterfaceSettings;
using InterfaceSettingsPtr = std::shared_ptr<InterfaceSettings>;

class InterfaceSettings {
public:
  void save(QXmlStreamWriter &xml);
  void load(QXmlStreamReader &xml);

  static std::string stringType(InterfaceType type);

  static InterfaceType type(std::string name);

  /* Creates an interface for the specified type. Returns
   * nullptr if the type is not supported */
  static InterfaceSettingsPtr create(InterfaceType type);

  InterfaceType type() { return type_; }

  std::string name() { return name_; }

  void setName(const std::string &name) { name_ = name; }

protected:
  virtual void saveCustom(QXmlStreamWriter &xml) = 0;
  /* Loads a custom element. */
  virtual void loadCustom(QXmlStreamReader &xml) = 0;

private:
  InterfaceType type_;
  std::string name_;
};

class SocketCanSettings : public InterfaceSettings {
public:
  void setInterface(const std::string &interface);
  std::string interface() { return scInterface_; }

protected:
  void saveCustom(QXmlStreamWriter &xml) override;

  void loadCustom(QXmlStreamReader &xml) override;

private:
  std::string scInterface_;
};

#endif // LIBRETUNER_INTERFACE_H
