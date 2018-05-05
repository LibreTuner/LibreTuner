/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *  
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
