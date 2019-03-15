#ifndef LT_LINKS_H
#define LT_LINKS_H

#include <vector>
#include <memory>
#include <functional>
#include <filesystem>

#include "util/signal.h"

#include "lt/link/datalink.h"

#include <QAbstractItemModel>

/**
 * Manages saving and loading of databases
 */
class Links : public QAbstractItemModel
{
public:
    Links() = default;
    Links(const Links&) = delete;
    Links &operator=(const Links&) = delete;
    Links(Links &&database);
    
    // Attempts to save link information to file
    void save() const;
    
    // Attempts to load link information from file and populated links
    void load();
    
    // Autodetects links
    void detect();
    
    // Adds a new datalink to the database
    void add(lt::DataLinkPtr &&link);

    // Get link at index. Detected links are indexed first.
    // Returns nullptr if the index is out of bounds
    lt::DataLink *get(std::size_t index) const;
    
    // Returns total amount of detected links
    inline std::size_t detectedCount() const { return detectedLinks_.size(); }
    
    // Returns total amount of manual links
    inline std::size_t manualCount() const { return manualLinks_.size(); }
    
    // Returns the total amount of links
    inline std::size_t count() const { return detectedCount() + manualCount(); }
    
    // Gets manual link at index.
    // Returns nullptr if the index is out of bounds
    lt::DataLink *getManual(std::size_t index) const;
    
    // Get detected link at index.
    // Returns nullptr if the index is out of bounds
    lt::DataLink *getDetected(std::size_t index) const;
    
    // Returns the first datalink
    lt::DataLink *getFirst() const;
    
    // Returns true if the database is empty
    bool empty() const { return count() == 0; }
    
    void setPath(const std::filesystem::path &path) { path_ = path; }
    const std::filesystem::path &path() const { return path_; }
    
    // Removes datalink from database
    void remove(lt::DataLink *link);
private:
    std::vector<lt::DataLinkPtr> manualLinks_;
    std::vector<lt::DataLinkPtr> detectedLinks_;
    
    std::filesystem::path path_;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

Q_DECLARE_METATYPE(lt::DataLink*)


class LinksListModel : public QAbstractListModel {
public:
    LinksListModel(const Links &links) : links_(links) {}

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    const Links &links_;
};

#endif // LT_LINKS_H
