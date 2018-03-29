#ifndef TUNE_H
#define TUNE_H

#include <memory>
#include <string>

/**
 * TODO: write docs
 */
class Tune
{
public:
    void setName(const std::string &name)
    {
        name_ = name;
    }
    
    const std::string &name() const
    {
        return name_;
    }
private:
    std::string name_;
};

typedef std::shared_ptr<Tune> TunePtr;

#endif // TUNE_H
