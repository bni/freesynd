#pragma once

/*
 * MIT License
 *
 * Copyright (c) 2020 Kaine (https://github.com/Kainev/EventManager)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cassert>
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>
#include <queue>


typedef std::uint32_t EventID;

class ListenerHandle
{
public:
    ListenerHandle(std::uint32_t sparse_index = 0u, std::uint32_t dense_index = 0u, std::uint32_t version = 0u, EventID event_id = 0u)
        : m_sparse_index(sparse_index),
          m_dense_index(dense_index),
          m_version(version),
          m_event_id(event_id) {}


public:
    std::uint32_t m_sparse_index;
    std::uint32_t m_dense_index;
    std::uint32_t m_version;
    EventID       m_event_id;


    friend class EventManager;

    template<typename T>
    friend struct CallbackContainer;
};


struct CallbackContainerBase
{
    virtual void remove_callback(const ListenerHandle& handle) = 0;
    virtual ~CallbackContainerBase() = default;
};

template<typename T>
struct CallbackContainer : CallbackContainerBase
{
    CallbackContainer(EventID an_event_id) : event_id{ an_event_id } {}

    std::vector<std::function<void(T*)>>    callbacks;
    std::vector<ListenerHandle>             handles;
    EventID                                 event_id;

    std::vector<ListenerHandle>             sparse;
    std::vector<std::uint32_t>              dense;
    std::queue<std::uint32_t>               free_sparse_indices;

    template<typename T_Function>
    auto add_callback(T_Function callback)->ListenerHandle;
    void remove_callback(const ListenerHandle& handle) override;
};


class EventManager
{
    using CallbackContainers = std::vector<std::unique_ptr<CallbackContainerBase>>;
public:
    template<typename T, typename T_Function>
    static auto listen(T_Function callback)->ListenerHandle;

    template<typename T, typename T_Instance, typename T_Function>
    static auto listen(T_Instance& instance, T_Function callback)->ListenerHandle;

    template<typename T, typename T_Instance, typename T_Function>
    static auto listen(T_Instance* instance, T_Function callback)->ListenerHandle;

    static void remove_listener(const ListenerHandle& handle);

    template<typename T, typename... T_Args>
    static void fire(T_Args...args);


private:
    template<typename T>
    static auto get_event_id()->EventID;

    template<typename T>
    static auto register_event()->EventID;

private:
    static inline CallbackContainers    s_callbacks;
    static inline EventID               s_next_event_id{ 0u };

    friend class ListenerHandle;
};


template<typename T, typename T_Function>
inline ListenerHandle EventManager::listen(T_Function callback)
{
    return static_cast<CallbackContainer<T>*>(s_callbacks[get_event_id<T>()].get())->add_callback(callback);
}

template<typename T, typename T_Instance, typename T_Function>
inline ListenerHandle EventManager::listen(T_Instance& instance, T_Function callback)
{
    return static_cast<CallbackContainer<T>*>(s_callbacks[get_event_id<T>()].get())->add_callback([&instance, callback](T* event) { (instance.*callback)(event); });
}

template<typename T, typename T_Instance, typename T_Function>
inline ListenerHandle EventManager::listen(T_Instance* instance, T_Function callback)
{
    return static_cast<CallbackContainer<T>*>(s_callbacks[get_event_id<T>()].get())->add_callback([instance, callback](T* event) { (instance->*callback)(event); });
}

inline void EventManager::remove_listener(const ListenerHandle& handle)
{
    s_callbacks[handle.m_event_id]->remove_callback(handle);
}

template<typename T, typename ...T_Args>
inline void EventManager::fire(T_Args ...args)
{
    T event{ args... };

    auto& callbacks = static_cast<CallbackContainer<T>*>(s_callbacks[get_event_id<T>()].get())->callbacks;
    for (auto& callback : callbacks)
        callback(&event);
}

template<typename T>
inline EventID EventManager::get_event_id()
{
    static EventID event_id = register_event<T>();
    return event_id;
}

template<typename T>
inline EventID EventManager::register_event()
{
    s_callbacks.emplace_back(std::make_unique<CallbackContainer<T*>>(s_next_event_id));
    return s_next_event_id++;
}


template<typename T>
template<typename T_Function>
inline auto CallbackContainer<T>::add_callback(T_Function callback) -> ListenerHandle
{
    std::uint32_t sparse_index;

    if (free_sparse_indices.empty())
    {
        sparse_index = callbacks.size();
        sparse.emplace_back(sparse_index, sparse_index, 0u, event_id);
    }
    else
    {
        sparse_index = free_sparse_indices.front();
        free_sparse_indices.pop();
    }

    dense.push_back(sparse_index);
    callbacks.emplace_back(callback);

    return sparse[sparse_index];
}


template<typename T>
inline void CallbackContainer<T>::remove_callback(const ListenerHandle& handle)
{
    assert(handle.m_version == sparse[handle.m_sparse_index].m_version);

    sparse[handle.m_sparse_index].m_version++;

    std::uint32_t remove_index = sparse[handle.m_sparse_index].m_dense_index;
    std::uint32_t last_index = callbacks.size() - 1;

    dense[remove_index] = dense[last_index];
    sparse[dense[remove_index]].m_dense_index = remove_index;

    std::swap(callbacks[remove_index], callbacks[last_index]);

    free_sparse_indices.push(handle.m_sparse_index);
    callbacks.pop_back();
    dense.pop_back();
}
