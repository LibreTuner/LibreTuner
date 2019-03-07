#include "linkdatabase.h"

#include "datalink.h"
#include "serializeddata.h"
#include "logger.h"

#include <fstream>

#ifdef WITH_J2534
#include "passthru.h"
#endif

#ifdef WITH_SOCKETCAN
#include "socketcan.h"
#endif



struct LinkData {
    std::string type;
    std::string name;
    std::string port;
};

namespace serialize {
    template<typename D>
    void deserialize(D &d, LinkData &link) {
        d.deserialize(link.type);
        d.deserialize(link.name);
        d.deserialize(link.port);
    }
    
    template<typename S>
    void serialize(S &s, const LinkData &link) {
        s.serialize(link.type);
        s.serialize(link.name);
        s.serialize(link.port);
    }
}


datalink::LinkDatabase::LinkDatabase() : updateSignal_(UpdateSignal::create()), removeSignal_(RemoveSignal::create())
{
    
}


void datalink::LinkDatabase::load()
{
    if (path_.empty()) {
        throw std::runtime_error("database path has not been set");
    }
    
    std::ifstream file(path_, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        // Does not exist
        return;
    }
    
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    std::vector<LinkData> links;
    
    serialize::Deserializer<serialize::InputBufferAdapter<std::vector<char>>> des(buffer);
    des.load(links);
    
    for (const LinkData &link : links) {
        if (link.type == "socketcan") {
#ifdef WITH_SOCKETCAN
            manualLinks_.emplace_back(std::make_unique<datalink::SocketCanLink>(link.name, link.port));
#else
            Logger::warning("SocketCAN is unuspported on this platform, ignoring link.");
#endif
        } else {
            throw std::runtime_error("Unknown datalink type: " + link.type);
        }
    }
    
    updateSignal_->call();
}


void datalink::LinkDatabase::save() const
{
    if (path_.empty()) {
        throw std::runtime_error("database path has not been set");
    }
    
    // Save manual links
    std::vector<LinkData> links;
    for (const datalink::LinkPtr &link : manualLinks_) {
        std::string type;
        if (link->type() == datalink::Type::SocketCan) {
            type = "socketcan";
        }
        LinkData data;
        data.type = type;
        data.name = link->name();
        data.port = link->port();
        links.emplace_back(std::move(data));
    }
    
    std::vector<char> buffer;
    serialize::Serializer<serialize::OutputBufferAdapter<std::vector<char>>> ser(buffer);
    ser.save(links);
    
    std::ofstream file(path_, std::ios::binary | std::ios::out);
    file.write(buffer.data(), buffer.size());
    file.close();
}


void datalink::LinkDatabase::detect()
{
    detectedLinks_.clear();
#ifdef WITH_J2534
    for (std::unique_ptr<datalink::PassThruLink> &link : datalink::detect_passthru_links()) {
        detectedLinks_.emplace_back(std::unique_ptr<datalink::Link>(static_cast<datalink::Link*>(link.release())));
    }
#endif
    updateSignal_->call();
}


void datalink::LinkDatabase::add(datalink::LinkPtr && link)
{
    manualLinks_.emplace_back(std::move(link));
    updateSignal_->call();
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


datalink::Link * datalink::LinkDatabase::getFirst() const
{
    return get(0);
}


void datalink::LinkDatabase::remove(datalink::Link* link)
{
    manualLinks_.erase(std::remove_if(manualLinks_.begin(), manualLinks_.end(), [link](const LinkPtr &l) { return l.get() == link; }));
    removeSignal_->call(link);
    updateSignal_->call();
}



datalink::LinkDatabase::LinkDatabase(datalink::LinkDatabase && database) : manualLinks_{std::move(database.manualLinks_)}, detectedLinks_{std::move(database.detectedLinks_)}
{

}
