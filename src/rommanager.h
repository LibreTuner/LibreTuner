#ifndef ROMMANAGER_H
#define ROMMANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

enum RomType
{
    ROM_NONE = 0,
    ROM_MAZDASPEED6,
};

/* Sub types are used for specific firmware versions */
enum RomSubType
{
    ROM_SUB_NONE = 0,
    ROM_SUB_L38K, // Mazdaspeed6 ECU model
};

/* ROM Metadata */
class RomData
{
public:
    void setName(const std::string &name)
    {
        name_ = name;
    }
    
    std::string name() const
    {
        return name_;
    }
    
    void setPath(const std::string &path)
    {
        path_ = path;
    }
    
    std::string path() const
    {
        return path_;
    }
    
    void setType(RomType type)
    {
        type_ = type;
    }
    
    RomType type() const
    {
        return type_;
    }
    
    void setSubType(RomSubType type)
    {
        subType_ = type;
    }
    
    RomSubType subType() const
    {
        return subType_;
    }
private:
    std::string name_;
    std::string path_;
    RomType type_ = ROM_NONE;
    RomSubType subType_ = ROM_SUB_NONE;
};

typedef std::shared_ptr<RomData> RomDataPtr;

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
    
    std::vector<RomDataPtr> &roms()
    {
        return roms_;
    }
    
    bool addRom(const std::string &name, RomType type, const uint8_t *data, size_t size);
    
    /* Analyzes the ROM data to determine the subtype. Returns
     * ROM_SUB_NONE if the subtype could not be determined. */
    RomSubType getSubType(RomType type, const uint8_t *data, size_t size);
    
private:
    RomManager();
    QString lastError_;
    std::vector<RomDataPtr> roms_;
    
    
    void readRoms(QXmlStreamReader &xml);
    
signals:
    void updateRoms();
};

#endif // ROMMANAGER_H
