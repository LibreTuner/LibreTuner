#ifndef LINKDATABASE_H
#define LINKDATABASE_H

#include <vector>
#include <memory>


namespace datalink {
    
class Link;
using LinkPtr = std::unique_ptr<Link>;

/**
 * Manages saving and loading of databases
 */
class LinkDatabase
{
public:
    LinkDatabase() = default;
    LinkDatabase(const LinkDatabase&) = delete;
    LinkDatabase &operator=(const LinkDatabase&) = delete;
    LinkDatabase(LinkDatabase &&database);
    
    // Attempts to save link information to file
    void save() const;
    
    // Attempts to load link information from file and populated links
    void load();
    
    // Autodetects links
    void detect();
    
    // Adds a new datalink to the database
    void add(LinkPtr &&link);

    // Get link at index. Detected links are indexed first.
    // Returns nullptr if the index is out of bounds
    Link *get(std::size_t index) const;
    
    // Returns total amount of detected links
    inline std::size_t detectedCount() const { return detectedLinks_.size(); }
    
    // Returns total amount of manual links
    inline std::size_t manualCount() const { return manualLinks_.size(); }
    
    // Returns the total amount of links
    inline std::size_t count() const { return detectedCount() + manualCount(); }
    
    // Gets manual link at index.
    // Returns nullptr if the index is out of bounds
    Link *getManual(std::size_t index) const;
    
    // Get detected link at index.
    // Returns nullptr if the index is out of bounds
    Link *getDetected(std::size_t index) const;
    
    void setPath(const std::string &path) { path_ = path; }
    const std::string &path() const { return path_; }
private:
    std::vector<LinkPtr> manualLinks_;
    std::vector<LinkPtr> detectedLinks_;
    
    std::string path_;
};

} // namespace datalink

#endif // DATALINKDATABASE_H
