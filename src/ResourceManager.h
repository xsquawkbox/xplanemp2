#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <functional>
#include <memory>
#include <future>
#include <chrono>
#include <unordered_map>
#include <assert.h>
//#include "XPLMUtilities.h"

template <typename T>
class ResourceManager
{
public:
    using ResourceHandle = std::shared_ptr<T>;
    using Future = std::future<ResourceHandle>;

    ResourceManager(std::function<Future(std::string)> factory) : m_factory(factory) {}

    std::shared_ptr<T> get(const std::string &name)
    {
        std::shared_ptr<T> resource;
        auto resourceIt = m_resourceCache.find(name);
        if (resourceIt != m_resourceCache.end())
        {
            resource = resourceIt->second.lock();
        }
        if (resource) { return resource; }

        auto futureIt = m_futureCache.find(name);
        if (futureIt == m_futureCache.end())
        {
            m_futureCache[name] = m_factory(name);
        }
        auto &future = m_futureCache[name];

        assert(future.valid());
        if (future.wait_for(std::chrono::duration<int>::zero()) != std::future_status::ready) // not yet finished loading
        {
            return nullptr;
        }
        resource = future.get();
        m_futureCache.erase(name);
        m_resourceCache[name] = resource;
        return resource;
    }

private:
    std::function<Future(std::string)> m_factory;
    std::unordered_map<std::string, std::weak_ptr<T>> m_resourceCache;
    std::unordered_map<std::string, Future> m_futureCache;
};

#endif
