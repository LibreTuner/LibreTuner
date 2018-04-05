#ifndef TUNEMANAGER_H
#define TUNEMANAGER_H

#include <memory>
#include <vector>

#include <QObject>
#include <QXmlStreamReader>


class Tune;
typedef std::shared_ptr<Tune> TunePtr;

class Rom;
typedef std::shared_ptr<Rom> RomPtr;

/**
 * Manages tunes and tune metadata
 */
class TuneManager : public QObject
{
    Q_OBJECT
public:
    /* Returns the global TuneManager object */
    static TuneManager *get();
    
    /* Loads tune metadata from storage. If unsuccessful,
     * returns false and sets lastError */
    bool load();
    
    /* Saves tune data. If unsuccessful, returns false and
     * sets lastError */
    bool save();
    
    
    QString lastError() const
    {
        return lastError_;
    }
    
    std::vector<TunePtr> &tunes()
    {
        return tunes_;
    }
    
    /* Creates a new tune with base 'base'. Returns the new tune.
     * If the tune could not be created, returns nullptr and sets lastError */
    TunePtr createTune(RomPtr base, const std::string &name);
    
private:
    TuneManager();
    
    void readTunes(QXmlStreamReader &xml);
    
    std::vector<TunePtr> tunes_;
    QString lastError_;
    
signals:
    void updateTunes();
};

#endif // TUNEMANAGER_H
