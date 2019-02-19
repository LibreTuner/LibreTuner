//
// Created by altenius on 12/20/18.
//

#ifndef LIBRETUNER_DATALINK_H
#define LIBRETUNER_DATALINK_H

#include <string>
#include <memory>

class CanInterface;
namespace isotp {
    class Protocol;
}

namespace datalink {
    enum class Type {
        SocketCan,
        PassThru,
        Invalid,
    };



    enum class Protocol {
        None = 0,
        Can = 0x1,
    };

    inline Protocol operator|(Protocol lhs, Protocol rhs) {
        using DType = std::underlying_type<Protocol>::type;
        return static_cast<Protocol>(static_cast<DType>(lhs) |
                                             static_cast<DType>(rhs));
    }

    inline Protocol operator&(Protocol lhs, Protocol rhs) {
        using DType = std::underlying_type<Protocol>::type;
        return static_cast<Protocol>(static_cast<DType>(lhs) &
                                             static_cast<DType>(rhs));
    }



    class Link {
    public:
        explicit Link(const std::string &name);
        virtual ~Link() = default;

        virtual Type type() const = 0;

        // Returns a bitflag of supported protocols
        virtual Protocol supported_protocols() const =0;

        // Returns the pretty name of the link for displaying to the user
        const std::string &name() const { return name_; }

        // Creates a CAN interface with the specified baudrate. The baudrate
        // may not be supported by the link. Returns nullptr if the interface is
        // not supported.
        virtual std::unique_ptr<CanInterface> can(uint32_t baudrate);

        // Currently a stub. Returns nullptr.
        virtual std::unique_ptr<isotp::Protocol> isotp();

    protected:
        std::string name_;
    };
}


#endif //LIBRETUNER_DATALINK_H
