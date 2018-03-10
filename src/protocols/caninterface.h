#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include <cstddef>
#include <cstdint>
#include <string>


class CanMessage
{
public:
    void setMessage(uint32_t id, const uint8_t *message, uint8_t length);
    
    CanMessage() {};
    CanMessage(uint32_t id, const uint8_t *message, uint8_t length);
    
    uint32_t id() const
    {
        return id_;
    }
    
    const uint8_t *message() const
    {
        return message_;
    }
    
    /* Returns a human-readable string representing the message data */
    std::string strMessage() const;
    
    uint8_t length() const
    {
        return messageLength_;
    }
    
private:
    uint32_t id_;
    uint8_t message_[8];
    uint8_t messageLength_;
};



/* Abstract CAN interface */
class CanInterface
{
public:
    enum CanError
    {
        ERR_SOCKET, // Socket creation error. err will be set
        ERR_READ, // Read error. err will be set
    };
    
    class Callbacks
    {
    public:
        /* Called when a new message is received */
        virtual void onRecv(const CanMessage &message) =0;
        
        /* Called when an error occurs. In some cases, err
         * will be set to an errno value. */
        virtual void onError(CanError error, int err) =0;
    };
    
    CanInterface(Callbacks& callbacks);
    virtual ~CanInterface() {};
    
    /* Send a CAN message. len should be no greater than 8.
     * Returns true if a message was sent */
    bool send(int id, const uint8_t *message, uint8_t len);
    
    virtual bool send(const CanMessage &message) =0;
    
    
    /* Receives a message and returns the size of the message.
     * message should be at least 8 bytes wide. Returns false
     * if an error occured while reading. */
    virtual bool recv(CanMessage &message) =0;
    
protected:
    Callbacks &callbacks_;
};

#endif // CANINTERFACE_H
