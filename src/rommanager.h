#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

#include "rom.h"

/**
 * Manages ROM files and metadata
 */
class RomManager : public QObject
{
    Q_OBJECT
public:
    static RomManager *get();
    
    /* Loads rom list and metadata. Returns true if no errors
     * occurred */
    bool load();
    
    /* Saves rom list and metadata */
    bool save();
    
    QString lastError() const
    {
        return lastError_;
    }
    
    /* Returns the amount of roms */
    size_t count() const
    {
        return roms_.size();
    }
    
    std::vector<RomPtr> &roms()
    {
        return roms_;
    }
    
    bool addRom(const std::string &name, DefinitionPtr definition, const uint8_t *data, size_t size);
    
    /* Returns the ROM with id or nullptr if the ROM does
     * not exist */
    RomPtr fromId(int id);
    
private:
    RomManager();
    QString lastError_;
    std::vector<RomPtr> roms_;
    int nextId_;
    
    
    void readRoms(QXmlStreamReader &xml);
    
signals:
    void updateRoms();
};

#endif // ROMMANAGER_H
