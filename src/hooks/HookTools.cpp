#include "common.hpp"
#include "HookTools.hpp"

namespace EC
{
struct EventCallbackData
{
    explicit EventCallbackData(EventFunction function, const std::string &name, enum ec_priority priority) : function{ function }, priority{ int(priority) }, event_name{ name }
    {
    }
    EventFunction function;
    int priority;
    std::string event_name;
};

static std::vector<EventCallbackData> events[ec_types::EcTypesSize];

void Register(enum ec_types type, const EventFunction &function, const std::string &name, enum ec_priority priority)
{
    events[type].emplace_back(function, name, priority);
    // Order vector to always keep priorities correct
    std::sort(events[type].begin(), events[type].end(), [](EventCallbackData &a, EventCallbackData &b) { return a.priority < b.priority; });
}

void Unregister(enum ec_types type, const std::string &name)
{
    auto &e = events[type];
    for (auto it = e.begin(); it != e.end(); ++it)
        if (it->event_name == name)
        {
            e.erase(it);
            break;
        }
}

void run(ec_types type)
{
    auto &vector = events[type];
    for (auto &i : vector)
    {
        i.function();
    }
}
} // namespace EC
