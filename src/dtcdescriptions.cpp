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

#include "dtcdescriptions.h"
#include "logger.h"

#include <QFile>
#include <QXmlStreamReader>

void DtcDescriptions::load()
{
    QFile file(":/codes.xml");
    if (!file.open(QFile::ReadOnly))
    {
        Logger::warning("Could not open :/codes.xml. DTC descriptions will not "
                        "be available");
        return;
    }

    QXmlStreamReader xml(&file);

    if (!xml.readNextStartElement())
    {
        Logger::warning(
            "Could not load DTC descriptions: no xml start element");
        return;
    }

    if (xml.name() != "codes")
    {
        Logger::warning(
            ("Could not load DTC descriptions: xml start element name was '" +
             xml.name() + "', expected 'codes'")
                .toStdString());
        return;
    }

    while (xml.readNextStartElement())
    {
        if (xml.name() != "code")
        {
            xml.raiseError("unexpected element. Expected 'code' tag");
            continue;
        }

        QXmlStreamAttributes attrs = xml.attributes();
        if (!attrs.hasAttribute("code"))
        {
            xml.raiseError("no code attribute");
            continue;
        }

        descriptions_.emplace(attrs.value("code").toString().toStdString(),
                              xml.readElementText().toStdString());
    }

    if (xml.hasError())
    {
        Logger::warning(QObject::tr("Failed to load DTC descriptions due to an "
                                    "xml error: %1\nLine %2, column %3")
                            .arg(xml.errorString())
                            .arg(xml.lineNumber())
                            .arg(xml.columnNumber())
                            .toStdString());
    }

    Logger::info("Loaded " + std::to_string(descriptions_.size()) +
                 " DTC descriptions");
}

std::pair<bool, std::string>
DtcDescriptions::get(const std::string & code) const
{
    auto it = descriptions_.find(code);
    if (it == descriptions_.end())
    {
        return std::make_pair(false, std::string());
    }

    return std::make_pair(true, it->second);
}
