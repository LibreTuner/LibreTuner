#include "rommanager.h"
#include "libretuner.h"

#include <QFileInfo>
#include <cassert>

#include <iostream>


RomManager * RomManager::get()
{
    static RomManager romManager;
    return &romManager;
}


bool RomManager::load()
{
    LibreTuner::get()->checkHome();
    
    QString listPath = LibreTuner::get()->home() + "/" + "roms.xml";
    
    if (!QFile::exists(listPath))
    {
        return true;
    }
    
    QFile listFile(listPath);
    if (!listFile.open(QFile::ReadOnly))
    {
        lastError_ = listFile.errorString();
        return false;
    }
    
    QXmlStreamReader xml(&listFile);
    
    if (xml.readNextStartElement()) 
    {
        if (xml.name() == "roms")
        {
            readRoms(xml);
        }
        else
        {
            xml.raiseError(QObject::tr("This file is not a ROM list document"));
        }
    }
    
    if (xml.error())
    {
        lastError_ = QObject::tr("%1\nLine %2, column %3")
                    .arg(xml.errorString())
                    .arg(xml.lineNumber())
                    .arg(xml.columnNumber());
        return false;
    }
    
    emit updateRoms();
    
    return true;
}



void RomManager::readRoms(QXmlStreamReader &xml)
{
    assert(xml.isStartElement() && xml.name() == "roms");
    roms_.clear();
    
    while (xml.readNextStartElement())
    {
        if (xml.name() != "rom")
        {
            xml.raiseError("Unexpected element in roms");
            return;
        }
        
        
        RomDataPtr rom = std::make_shared<RomData>();
        
        // Read ROM data
        while (xml.readNextStartElement())
        {
            if (xml.name() == "name")
            {
                rom->setName(xml.readElementText().trimmed().toStdString());
            }
            else if (xml.name() == "path")
            {
                rom->setPath(xml.readElementText().trimmed().toStdString());
            }
            else if (xml.name() == "type")
            {
                QString type = xml.readElementText().toLower();
                if (type == "mazdaspeed6")
                {
                    rom->setType(ROM_MAZDASPEED6);
                }
                else
                {
                    xml.raiseError("Unknown ROM type");
                }
            }
            else if (xml.name() == "subtype")
            {
                QString type = xml.readElementText().toLower();
                if (type == "l38k")
                {
                    rom->setSubType(ROM_SUB_L38K);
                }
                else
                {
                    xml.raiseError("Unknown ROM subtype");
                }
            }
        }
        
        // Verifications
        if (rom->name().empty())
        {
            xml.raiseError("ROM name is empty");
        }
        if (rom->path().empty())
        {
            xml.raiseError("ROM path is empty");
        }
        if (rom->type() == ROM_NONE)
        {
            xml.raiseError("ROM type is empty");
        }
        if (rom->subType() == ROM_SUB_NONE)
        {
            xml.raiseError("ROM subtype is empty");
        }
        
        if (xml.hasError())
        {
            return;
        }
        
        roms_.push_back(rom);
    }
}




bool RomManager::save()
{
    LibreTuner::get()->checkHome();
    
    QString listPath = LibreTuner::get()->home() + "/" + "roms.xml";
    
    QFile listFile(listPath);
    if (!listFile.open(QFile::WriteOnly))
    {
        lastError_ = listFile.errorString();
        return false;
    }
    
    QXmlStreamWriter xml(&listFile);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-4); // tabs > spaces
    
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE roms>");
    xml.writeStartElement("roms");
    for (const RomDataPtr rom : roms_)
    {
        xml.writeStartElement("rom");
        xml.writeTextElement("name", QString::fromStdString(rom->name()));
        xml.writeTextElement("path", QString::fromStdString(rom->path()));
        switch(rom->type())
        {
            case ROM_MAZDASPEED6:
                xml.writeTextElement("type", "MAZDASPEED6");
                break;
        }
        switch(rom->subType())
        {
            case ROM_SUB_L38K:
                xml.writeTextElement("subtype", "L38K");
                break;
        }
        xml.writeEndElement();
    }
    
    xml.writeEndDocument();
    return true;
}



RomSubType RomManager::getSubType(RomType type, const uint8_t* data, size_t size)
{
    switch (type)
    {
        case ROM_MAZDASPEED6:
            // Check L38K. Offset 0x2a7c8 will be L38K
            if (memcmp(data + 0x2A7C8, "L38K", 4) == 0)
            {
                return ROM_SUB_L38K;
            }
            // TODO: add the other models
            break;
            
        default:
            break;
    }
    
    return ROM_SUB_NONE;
}



bool RomManager::addRom(const std::string& name, RomType type, const uint8_t* data, size_t size)
{
    LibreTuner::get()->checkHome();
    
    QString romRoot = LibreTuner::get()->home() + "/roms/";
    QString path = QString::fromStdString(name);
    if (QFile::exists(path))
    {
        int count = 0;
        do
        {
            path = QString::fromStdString(name) + QString::number(++count);
        } while (QFile::exists(romRoot + path));
    }
    
    QFile file(romRoot + path);
    if (!file.open(QFile::WriteOnly))
    {
        lastError_ = file.errorString();
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data), size);
    file.close();
    
    // Determine the subtype
    RomSubType subtype = getSubType(type, data, size);
    if (subtype == ROM_SUB_NONE)
    {
        lastError_ = "Unknown firmware version or this is the wrong vehicle. If this is the correct vehicle, please submit a bug report so we can add support for this firmware version.";
        return false;
    }
    
    RomDataPtr rom = std::make_shared<RomData>();
    rom->setName(name);
    rom->setPath(path.toStdString());
    rom->setType(type);
    rom->setSubType(subtype);
    roms_.push_back(rom);
    
    emit updateRoms();
    
    return save();
}



RomManager::RomManager()
{
}
