#ifndef TUNE_H
#define TUNE_H

#include <memory>
#include <string>

#include <QXmlStreamReader>


class Rom;
typedef std::shared_ptr<Rom> RomPtr;

class RomData;
typedef std::shared_ptr<RomData> RomDataPtr;

class TableGroup;
typedef std::shared_ptr<TableGroup> TableGroupPtr;


/**
 * TODO: write docs
 */
class Tune
{
public:
    void setName(const std::string &name)
    {
        name_ = name;
    }
    
    const std::string &name() const
    {
        return name_;
    }
    
    void setPath(const std::string &path)
    {
        path_ = path;
    }
    
    const std::string &path() const
    {
        return path_;
    }
    
    RomPtr base()
    {
        return base_;
    }
    
    void setBase(RomPtr base)
    {
        base_ = base;
    }
private:
    std::string name_;
    std::string path_;
    
    RomPtr base_;
};
typedef std::shared_ptr<Tune> TunePtr;





class TuneData
{
public:
    TuneData(TunePtr tune);
    
    /* Applies table modifications to data and computes checksums.
     * Returns false on error and sets lastError. */
    bool apply(uint8_t *data, size_t length);
    
    bool valid() const
    {
        return valid_;
    }
    
    std::string lastError() const
    {
        return lastError_;
    }
    
    TableGroupPtr tables()
    {
        return tables_;
    }
    
    TunePtr tune()
    {
        return tune_;
    }
    
    RomDataPtr romData()
    {
        return rom_;
    }
    
    bool save();
    
private:
    TunePtr tune_;
    RomDataPtr rom_;
    
    std::string lastError_;
    
    bool valid_;
    
    TableGroupPtr tables_;
    
    void readTables(QXmlStreamReader &xml);
};
typedef std::shared_ptr<TuneData> TuneDataPtr;

#endif // TUNE_H
