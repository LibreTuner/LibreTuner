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

#include "interface.h"

#include <algorithm>

void InterfaceSettings::save(QXmlStreamWriter &xml) {
    xml.writeStartElement("interface");
    xml.writeAttribute("type", QString::fromStdString(stringType(type_)));
    xml.writeTextElement("name", QString::fromStdString(name_));
    saveCustom(xml);
    xml.writeEndElement();
}

void InterfaceSettings::load(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            name_ = xml.readElementText().toStdString();
        } else {
            loadCustom(xml);
        }
    }
}

std::string InterfaceSettings::stringType(InterfaceType type) {
    switch (type) {
    case InterfaceType::SocketCan:
        return "socketcan";
    case InterfaceType::J2534:
        return "j2534";
    default:
        return "invalid";
    }
}

InterfaceType InterfaceSettings::type(std::string name) {
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (name == "socketcan") {
        return InterfaceType::SocketCan;
    }
    if (name == "j2534") {
        return InterfaceType::J2534;
    }
    return InterfaceType::Invalid;
}

InterfaceSettingsPtr InterfaceSettings::create(InterfaceType type) {
    switch (type) {
    case InterfaceType::SocketCan:
        return std::make_shared<SocketCanSettings>();
    case InterfaceType::J2534:
        return std::make_shared<J2534Settings>();
    default:
        return nullptr;
    }
}

void SocketCanSettings::setInterface(const std::string &interface) {
    scInterface_ = interface;
}

void SocketCanSettings::saveCustom(QXmlStreamWriter &xml) {
    xml.writeTextElement("interface", QString::fromStdString(scInterface_));
}

void SocketCanSettings::loadCustom(QXmlStreamReader &xml) {
    if (xml.name() == "interface") {
        scInterface_ = xml.readElementText().toStdString();
    }
}

void J2534Settings::setInterface(const j2534::J2534Ptr &j2534) {
    j2534_ = j2534;
}

void J2534Settings::saveCustom(QXmlStreamWriter &xml) {}

void J2534Settings::loadCustom(QXmlStreamReader &xml) {}
