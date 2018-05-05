#include "definitionmanager.h"
#include "libretuner.h"
#include "definition.h"

#include <QString>
#include <QDir>
#include <QFile>

DefinitionManager * DefinitionManager::get()
{
     static DefinitionManager definitionmanager;
    return &definitionmanager;
}



DefinitionManager::DefinitionManager()
{
}



DefinitionPtr DefinitionManager::getDefinition(const std::string& id)
{
    for (auto it = definitions_.begin(); it != definitions_.end(); ++it)
    {
        if ((*it)->id() == id)
        {
            return *it;
        }
    }
    return nullptr;
}



bool DefinitionManager::load()
{
    LibreTuner::get()->checkHome();
    
    QString listPath = LibreTuner::get()->home() + "/definitions";
    
    if (!QFile::exists(listPath))
    {
        return true;
    }
    
    QDir defsDir(listPath);
    defsDir.setFilter(QDir::NoFilter);
    for (QFileInfo &info : defsDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::NoSort))
    {
        if (info.isDir())
        {
            DefinitionPtr def = std::make_shared<Definition>();
            if (!def->load(info.filePath()))
            {
                lastError_ = def->lastError();
                return false;
            }
            definitions_.push_back(def);
        }
    }
    
    return true;
}
