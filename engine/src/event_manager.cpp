// event_manager.cpp

#pragma once

#include "application.hpp"
#include "gameobject_manager.hpp"
#include "event_manager.hpp"
#include "buffer.hpp"

using namespace types;

namespace realware
{
    cEvent::cEvent(eEvent type, cGameObject* receiver, EventFunction&& function) : _receiver(receiver), _type(type), _function(std::make_shared<EventFunction>(std::move(function)))
    {
    }

    void cEvent::Invoke(cBuffer* data)
    {
        _function->operator()(data);
    };

    mEvent::mEvent(cContext* context) : iObject(context) {}

    void mEvent::Unsubscribe(eEvent type, cGameObject* receiver)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        for (usize i = 0; i < events->GetElementCount(); i++)
        {
            const cEvent* listenerEvent = &events->GetElements()[i];
            const cGameObject* listenerReceiver = listenerEvent->GetReceiver();
            if (listenerReceiver == receiver)
            {
                events->Delete(listenerReceiver->GetID());
                return;
            }
        }
    }

    void mEvent::Send(eEvent type)
    {
        cBuffer data;

        Send(type, &data);
    }

    void mEvent::Send(eEvent type, cBuffer* data)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        for (usize i = 0; i < events->GetElementCount(); i++)
            events->GetElements()[i].Invoke(data);
    }
}