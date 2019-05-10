#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>

#include <sol.hpp>

class PluginState;

/**
 * @todo write docs
 */
class Plugin {
public:
    // Loads a plugin. Throws an exception on failure
    Plugin(PluginState &state, const std::string &path);

    static void loadLibrary(sol::state &state);

    // API
    static void l_register(const std::string &name);

    // Prevent copies
    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;

private:
    PluginState *state_;
    std::string name_;
};

#endif // PLUGIN_H
