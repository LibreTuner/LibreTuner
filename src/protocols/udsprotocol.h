#ifndef UDSPROTOCOL_H
#define UDSPROTOCOL_H

#include <string>
#include <memory>
#include <vector>

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

/* Request SIDs */
#define UDS_REQ_SESSION 0x10
#define UDS_REQ_SECURITY 0x27
#define UDS_REQ_READMEM 0x23

/* Reponse SIDs */
#define UDS_RES_SESSION 0x50
#define UDS_RES_SECURITY 0x67
#define UDS_RES_NEGATIVE 0x7F
#define UDS_RES_READMEM 0x63






class UdsResponse {
public:
    UdsResponse() {};
    UdsResponse(uint8_t id, const uint8_t *message, size_t length);
    
    uint8_t id() const
    {
        return responseId_;
    }
    
    void setId(uint8_t id)
    {
        responseId_ = id;
    }
    
    size_t length() const
    {
        return message_.size();
    }
    
    uint8_t &operator[](int index)
    {
        return message_[index];
    }
    
    
    const uint8_t *message() const
    {
        return message_.data();
    }
    
    const uint8_t &operator[](int index) const
    {
        return message_[index];
    }
    
    void setMessage(const uint8_t *message, size_t length);
    
private:
    uint8_t responseId_ = 0;
    std::vector<uint8_t> message_;
};



class UdsProtocol {
public:
    class Callbacks {
    public:
        virtual void onError(const std::string &error) =0;
        
        virtual void onRecv(const UdsResponse &response) =0;
        
        /* Called when a request times out */
        virtual void onTimeout() =0;
    };
    
    /* Create an interface with an ISO-TP layer */
    static std::shared_ptr<UdsProtocol> create(Callbacks *callbacks, CanInterfacePtr can, int srcId = 0x7E0, int dstId = 0x7E8);
    
    /* Sends a request. Returns false on failure. */
    virtual bool request(const uint8_t *message, size_t length) =0;
    
    /* Sends a DiagnosticSessionControl request */
    bool requestSession(uint8_t type);
    
    bool requestSecuritySeed();
    
    bool requestSecurityKey(const uint8_t *key, uint8_t length);
    
    /* ReadMemoryByAddress */
    bool requestReadMemoryAddress(uint32_t address, uint16_t length);
    
    
    virtual ~UdsProtocol() {};
    
protected:
    UdsProtocol(Callbacks *callbacks);
    
    
    Callbacks *callbacks_;
};


#endif // UDSPROTOCOL_H
