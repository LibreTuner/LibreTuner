#ifndef J2534MANAGER_H
#define J2534MANAGER_H

#include <memory>
#include <vector>

class J2534;
using J2534Ptr = std::shared_ptr<J2534>;

class J2534Manager
{
public:
    J2534Manager();
    J2534Manager(J2534Manager&&) = delete;
    J2534Manager(const J2534Manager&) = delete;

    // Returns the singleton
    static J2534Manager &get();

    void init();

    // Empties the vector of discovered interfaces and repopulates.
    void rediscover();

    const std::vector<const J2534Ptr> &interfaces() const {
        return interfaces_;
    }

private:
    std::vector<J2534Ptr> interfaces_;
};

#endif // J2534MANAGER_H
