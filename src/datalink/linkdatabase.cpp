#include "linkdatabase.h"

#include "datalink.h"

#ifdef WITH_J2534
#include "passthru.h"
#endif



void datalink::LinkDatabase::detect()
{
#ifdef WITH_J2534
    for (std::unique_ptr<datalink::PassThruLink> &link : datalink::detect_passthru_links()) {
        detectedLinks_.push_back(std::unique_ptr<datalink::Link>(static_cast<datalink::Link*>(link.release())));
    }
#endif
}


void datalink::LinkDatabase::add(datalink::LinkPtr && link)
{
    manualLinks_.emplace_back(std::move(link));
}



datalink::Link * datalink::LinkDatabase::get(std::size_t index) const
{
    if (index < 0 || index >= count()) {
        return nullptr;
    }
    
    if (index < detectedLinks_.size()) {
        return detectedLinks_[index].get();
    }
    return manualLinks_[index - detectedLinks_.size()].get();
}



datalink::Link * datalink::LinkDatabase::getDetected(std::size_t index) const
{
    if (index < 0 || index >= detectedCount()) {
        return nullptr;
    }
    
    return detectedLinks_[index].get();
}



datalink::Link * datalink::LinkDatabase::getManual(std::size_t index) const
{
    if (index < 0 || index >= manualCount()) {
        return nullptr;
    }
    
    return manualLinks_[index].get();
}



datalink::LinkDatabase::LinkDatabase(datalink::LinkDatabase && database) : manualLinks_{std::move(database.manualLinks_)}, detectedLinks_{std::move(database.detectedLinks_)}
{

}
