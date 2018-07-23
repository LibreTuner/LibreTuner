#ifndef J2534_H
#define J2534_H

#include <memory>
#include <string>

#include "datalink.h"

#include <windows.h>

struct J2534Info {
    std::string name;
    // Supported protocols
    DataLinkProtocol protocols;
    // DLL path
    std::string functionLibrary;
};



// J2534 API
using PassThruConnect = long (*) (unsigned long, unsigned long, unsigned long*);
using PassThruDisconnect = long (*) (unsigned long);
using PassThruReadMsgs = long (*) (unsigned long, PASSTHRU_MSG *, unsigned long*, unsigned long);



class J2534;
using J2534Ptr = std::shared_ptr<J2534>;

class J2534
{
public:
    // Initializes the interface by loading the DLL. May throw an exception
    void init();

    // Returns true if the interface's library has been loaded
    bool initialized() const;

    std::string name() const { return info_.name; }

    // Returns the protocols supported by the J2534 interface
    DataLinkProtocol protocols() const { return info_.protocols; }

    // Creates a J2534 interface. Must be initialized with init() before use.
    static J2534Ptr create(J2534Info &&info);

protected:
    J2534(J2534Info &&info) : info_(std::move(info)) {}

private:
    J2534Info info_;

    HINSTANCE hDll_;

    // Loads the dll
    void load();
};

#endif // J2534_H
