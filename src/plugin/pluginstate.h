#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <vector>

#include "plugin.h"

/**
 * @todo write docs
 */
class PluginState
{
public:
    PluginState();

    // Loads a script from a file
    void loadFile(const std::string & filename);

private:
    std::vector<Plugin> plugins_;

    //sol::state lua_;
};

#endif // PLUGINLOADER_H
