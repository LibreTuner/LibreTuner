#include "tabledefinitions.h"
#include "definition.h"


#include <cassert>

/*
TableDefinition::TableDefinition(int id, const std::string& name, const std::string &description, TableType type, TableCategory category, DataType dataType, uint32_t sizeX, const TableAxis *axisX, uint32_t sizeY, const TableAxis *axisY, int min, int max) : id_(id), name_(name), description_(description), type_(type), category_(category), dataType_(dataType), sizeX_(sizeX), sizeY_(sizeY), max_(max), min_(min), axisX_(axisX), axisY_(axisY)
{
}
*/



class LinearAxis : public TableAxis
{
public:
    LinearAxis(const std::string &name, const std::string &id, int iId, double start, double increment) : TableAxis(name, id, iId), start_(start), increment_(increment) {};
    
    double label(int idx) const override
    {
        return start_ + (idx * increment_);
    }
    
private:
    double start_;
    double increment_;
};



TableAxisPtr TableAxis::load(QXmlStreamReader& xml, int iId)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (!attributes.hasAttribute("id"))
    {
        xml.raiseError("No id attribute defined for axis");
        return nullptr;
    }
    if (!attributes.hasAttribute("type"))
    {
        xml.raiseError("No type attribute defined for axis");
        return nullptr;
    }
    
    QStringRef id = attributes.value("id");
    QStringRef type = attributes.value("type");
    
    if (type != "linear")
    {
        xml.raiseError("Unknown type attribute");
        return nullptr;
    }
    
    std::string name;
    double minimum, increment;
    bool foundMinimum = false, foundIncrement = false;
    
    bool ok;
    
    while (xml.readNextStartElement())
    {
        if (xml.name() == "name")
        {
            name = xml.readElementText().trimmed().toStdString();
        }
        else if (xml.name() == "minimum")
        {
            minimum = xml.readElementText().toDouble(&ok);
            if (!ok)
            {
                xml.raiseError("Failed to read minimum: not a number");
                return nullptr;
            }
            foundMinimum = true;
        }
        else if (xml.name() == "increment")
        {
            increment = xml.readElementText().toDouble(&ok);
            if (!ok)
            {
                xml.raiseError("Failed to read increment: not a number");
                return nullptr;
            }
            foundIncrement = true;
        }
        else
        {
            xml.raiseError("Unexpected element");
            return nullptr;
        }
    }
    
    if (type == "linear")
    {
        if (!foundMinimum)
        {
            xml.raiseError("No minimum element defined");
            return nullptr;
        }
        if (!foundIncrement)
        {
            xml.raiseError("No increment element defined");
            return nullptr;
        }
        
        return std::make_shared<LinearAxis>(name, id.toString().toStdString(), iId, minimum, increment);
    }
    
    return nullptr;
}



TableDefinition::TableDefinition(Definition *definition) : definition_(definition), sizeX_(1), sizeY_(1), min_(std::numeric_limits<int>::min()), max_(std::numeric_limits<int>::max())
{
}



TableDefinition::TableDefinition() : definition_(nullptr)
{
}



bool TableDefinition::load(QXmlStreamReader& xml, Definition *def)
{
    QXmlStreamAttributes attributes = xml.attributes();
    if (!attributes.hasAttribute("type"))
    {
        xml.raiseError("No type attribute defined");
        return false;
    }
    if (!attributes.hasAttribute("id"))
    {
        xml.raiseError("No id attribute defined");
        return false;
    }
    
    QStringRef sType = attributes.value("type");
    if (sType == "1d")
    {
        type_ = TABLE_1D;
    }
    else if (sType == "2d")
    {
        type_ = TABLE_2D;
    }
    else if (sType == "3d")
    {
        type_ = TABLE_3D;
    }
    else
    {
        xml.raiseError("Invalid type attribute. Accepted values: 1d, 2d, 3d");
        return false;
    }
    
    QStringRef sId = attributes.value("id");
    bool ok;
    id_ = sId.toInt(&ok);
    if (!ok)
    {
        xml.raiseError("Invalid id attribute: not a number");
        return false;
    }
    
    if (id_ > MAX_TABLEID)
    {
        xml.raiseError("id attribute exceeds maximum value (" + QString::number(MAX_TABLEID) + ")");
        return false;
    }

    bool foundName = false, foundDesc = false, foundCategory = false, foundDataType = false;
    
    while (xml.readNextStartElement())
    {
        if (xml.name() == "name")
        {
            name_ = xml.readElementText().trimmed().toStdString();
            foundName = true;
        }
        else if (xml.name() == "description")
        {
            description_ = xml.readElementText().trimmed().toStdString();
            foundDesc = true;
        }
        else if (xml.name() == "datatype")
        {
            QString sDataType = xml.readElementText().trimmed().toLower();
            if (sDataType == "uint8")
            {
                dataType_ = TDATA_UINT8;
            }
            else if (sDataType == "uint16")
            {
                dataType_ = TDATA_UINT16;
            }
            else if (sDataType == "uint32")
            {
                dataType_ = TDATA_UINT32;
            }
            else if (sDataType == "float")
            {
                dataType_ = TDATA_FLOAT;
            }
            else if (sDataType == "int8")
            {
                dataType_ = TDATA_INT8;
            }
            else if (sDataType == "int16")
            {
                dataType_ = TDATA_INT16;
            }
            else if (sDataType == "int32")
            {
                dataType_ = TDATA_INT32;
            }
            else
            {
                xml.raiseError("Invalid datatype.");
                return false;
            }
            foundDataType = true;
        }
        else if (xml.name() == "category")
        {
            QString sCategory = xml.readElementText().toLower();
            if (sCategory == "limiters")
            {
                category_ = TCAT_LIMITER;
            }
            else
            {
                category_ = TCAT_MISC;
            }
            foundCategory = true;
        }
        else if ((type_ == TABLE_1D && xml.name() == "size") || xml.name() == "sizex")
        {
            sizeX_ = xml.readElementText().toInt(&ok);
            if (!ok)
            {
                xml.raiseError("Invalid sizex element: not a number");
                return false;
            }
        }
        else if (xml.name() == "sizey")
        {
            sizeY_ = xml.readElementText().toInt(&ok);
            if (!ok)
            {
                xml.raiseError("Invalid sizey element: not a number");
                return false;
            }
        }
        else if (xml.name() == "axisx")
        {
            axisX_ = xml.readElementText().trimmed().toStdString();
        }
        else if (xml.name() == "axisy")
        {
            axisY_ = xml.readElementText().trimmed().toStdString();
        }
        else if (xml.name() == "minimum")
        {
            min_ = xml.readElementText().toDouble(&ok);
            if (!ok)
            {
                xml.raiseError("Invalid minimum element: not a number");
                return false;
            }
        }
        else if (xml.name() == "maximum")
        {
            max_ = xml.readElementText().toDouble(&ok);
            if (!ok)
            {
                xml.raiseError("Invalid maximum element: not a number");
                return false;
            }
        }
        else
        {
            xml.raiseError("Unexpected element");
            return false;
        }
    }
    
    if (!foundName)
    {
        xml.raiseError("No name element is defined");
        return false;
    }
    if (!foundCategory)
    {
        xml.raiseError("No category element is defined");
        return false;
    }
    if (!foundDataType)
    {
        xml.raiseError("No datatype element is defined");
        return false;
    }
    
    return true;
}



const TableAxis *TableDefinition::axisX() const
{
    return definition_->getAxis(axisX_);
}



const TableAxis *TableDefinition::axisY() const
{
    return definition_->getAxis(axisY_);
}



void TableDefinitions::addTable(TableDefinition &&table)
{
    int id = table.id();
    if (tables_.size() < id + 1)
    {
        tables_.resize(id + 1);
    }
    tables_[id] = std::move(table);
}



const TableDefinition *TableDefinitions::at(int id) const
{
    assert(id < count());
    return &definitions()[id];
}
