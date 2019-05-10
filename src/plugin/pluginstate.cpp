#include "pluginstate.h"

#include "logger.h"

PluginState::PluginState() {
    // Open libraries
    lua_.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine,
                        sol::lib::string, sol::lib::os, sol::lib::math,
                        sol::lib::table, sol::lib::debug, sol::lib::bit32,
                        sol::lib::io);
}

void PluginState::loadFile(const std::string &filename) {
    try {
        lua_.safe_script_file(filename);
    } catch (const sol::error &err) {
        Logger::warning("Error loading " + filename + ": " + err.what());
    }
}
