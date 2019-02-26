#ifndef DATALOGDATABASE_H
#define DATALOGDATABASE_H

#include "datalog.h"

#include <filesystem>

/**
 * Handles saving & loading of datalogs
 */
class DataLogDatabase
{
public:
    // Loads all logs
    void load();
    
    // Saves a datalog and adds it to the database. Returns a reference to the added log
    DataLog &saveLog(DataLog &&log);
    
private:
    std::vector<DataLog> datalogs_;
};

#endif // DATALOGDATABASE_H
