#include "tunemanager.h"
#include "libretuner.h"
#include "tune.h"

#include <cassert>

TuneManager * TuneManager::get()
{
    static TuneManager manager;
    return &manager;
}



TuneManager::TuneManager()
{
}



void TuneManager::readTunes(QXmlStreamReader& xml)
{
    assert(xml.isStartElement() && xml.name() == "tunes");
    tunes_.clear();
    
    while (xml.readNextStartElement())
    {
        if (xml.name() != "tune")
        {
            xml.raiseError("Unexpected element in tunes");
            return;
        }
        
        
        TunePtr rom = std::make_shared<Tune>();
        
        // Read ROM data
        while (xml.readNextStartElement())
        {
            if (xml.name() == "name")
            {
                
            }
        }
    }
}



bool TuneManager::save()
{
}



bool TuneManager::load()
{
    LibreTuner::get()->checkHome();
    
    QString listPath = LibreTuner::get()->home() + "/" + "tunes.xml";
    
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
        if (xml.name() == "tunes")
        {
            readTunes(xml);
        }
        else
        {
            xml.raiseError(QObject::tr("This file is not a tune list document"));
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
    
    emit updateTunes();
    
    return true;
}
