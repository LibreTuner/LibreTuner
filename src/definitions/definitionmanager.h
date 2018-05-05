#ifndef DEFINITIONMANAGER_H
#define DEFINITIONMANAGER_H

#include <string>
#include <vector>
#include <memory>

#include <QString>

class Definition;
typedef std::shared_ptr<Definition> DefinitionPtr;

/**
 * @todo write docs
 */
class DefinitionManager
{
public:
    static DefinitionManager *get();
    
    /* Returns the definition with the specified id. Returns
     * nullptr is no such definition exists. */
    DefinitionPtr getDefinition(const std::string &id);
    
    /* Loads rom list and metadata. Returns true if no errors
     * occurred */
    bool load();
    
    std::string lastError() const
    {
        return lastError_;
    }
    
    DefinitionPtr *definitions()
    {
        return definitions_.data();
    }
    
    size_t count() const
    {
        return definitions_.size();
    }
    
private:
    DefinitionManager();
    
    std::string lastError_;
    std::vector<DefinitionPtr> definitions_;
};

#endif // DEFINITIONMANAGER_H
