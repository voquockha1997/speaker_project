#pragma once
#include <event2/event.h>
#include <stdexcept>
#include "Utility/Singleton.hpp"
#include <signal.h>

namespace Net {
    class EventBase : public Singleton<EventBase>
    {
        friend class Singleton<EventBase>;
    public:
        bool doLoop();
        void forceStop();
        void StopSuccess();
        event_base *getEvtBase();
    protected:
        explicit EventBase();
    private:
        std::unique_ptr<struct event_base, void(*)(struct event_base*)> eventBase_;
        struct timeval tv = {1,0};
    };
}

