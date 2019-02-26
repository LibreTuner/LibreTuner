#include "datalogdatabase.h"
#include "libretuner.h"

#include <filesystem>

void DataLogDatabase::load()
{
    std::string directory = LT()->dataDirectory() + "/datalogs";
    if (!std::filesystem::create_directories(directory)) {
        throw std::runtime_error("Failed to create datalogs directory " + directory);
    }
}



DataLog & DataLogDatabase::saveLog(DataLog && log)
{
    std::string directory = LT()->dataDirectory() + "/datalogs";
    
    datalogs_.emplace_back(std::move(log));
    return datalogs_[datalogs_.size()-1];
}
