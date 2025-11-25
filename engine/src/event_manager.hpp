// event_manager.hpp

#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    class cApplication;
    class cGameObject;
    class cBuffer;
    
    enum class eEvent
    {
        NONE,
        KEY_PRESS
    };

    using EventFunction = std::function<void(cBuffer* const data)>;

    class cEvent
    {
        friend class mEvent;

    public:
        cEvent(eEvent type, cGameObject* receiver, EventFunction&& function);
        ~cEvent() = default;

        void Invoke(cBuffer* data);
        inline cGameObject* GetReceiver() const { return _receiver; }
        inline eEvent GetEventType() const { return _type; }
        inline std::shared_ptr<EventFunction> GetFunction() const { return _function; }

    private:
        eEvent _type = eEvent::NONE;
        cGameObject* _receiver = nullptr;
        mutable std::shared_ptr<EventFunction> _function;
    };

    class mEvent : public iObject
    {
    public:
        explicit mEvent(cContext* context);
        ~mEvent() = default;

        inline virtual cType GetType() const override final { return cType("EventManager"); }
        
        template <typename... Args>
        void Subscribe(const std::string& id, eEvent type, Args... args);
        void Unsubscribe(eEvent type, cGameObject* receiver);
        void Send(eEvent type);
        void Send(eEvent type, cBuffer* data);

    private:
        std::unordered_map<eEvent, std::shared_ptr<cIdVector<cEvent>>> _listeners;
    };

    template <typename... Args>
    void mEvent::Subscribe(const std::string& id, eEvent type, Args... args)
    {
        const auto listener = _listeners.find(type);
        if (listener == _listeners.end())
            _listeners.insert({ type, std::make_shared<cVector<cEvent>>(GetApplication()) });
        _listeners[type]->Add(id, type, std::forward<Args>(args)...);
    }
}