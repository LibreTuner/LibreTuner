#ifndef LT_EVENT_H
#define LT_EVENT_H

#include <memory>
#include <forward_list>
#include <functional>

namespace lt {
  
template<typename ...Args>
class EventConnection {
public:
    using Func_t = std::function<void(Args...)>;
    
    
};

template<typename ...Args>
class EventState {
public:
    using Connection_t = EventConnection<Args...>;
    
    void dispatch();
    
private:
    
};

    
template<typename ...Args>
class Event {
public:
    using State_t = EventState<Args...>;
    using Connection_t = typename State_t::Connection_t;
    
    void operator()() {
        
    }
    
private:
    
};
    
}

#endif
