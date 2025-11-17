// event_manager.cpp

#pragma once

#include "application.hpp"
#include "gameobject_manager.hpp"
#include "event_manager.hpp"

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace types;

    namespace utils
    {
        cEvent::cEvent(const eEventType& type, const EventFunction& function) : _type(type), _function(function)
        {
        }

        void cEvent::Invoke(sEventData* const data)
        {
            _function(data);
        };

        mEventManager::mEventManager(const cApplication* const app) : _app((cApplication*)app)
        {
        }

        void mEventManager::Subscribe(const cGameObject* receiver, cEvent& event)
        {
            event._receiver = (cGameObject*)receiver;

            eEventType eventType = event.GetType();
            auto listener = _listeners.find(eventType);
            if (listener == _listeners.end())
                _listeners.insert({ eventType, {}});
            _listeners[eventType].push_back(event);
        }

        void mEventManager::Send(const eEventType& type)
        {
            sEventData data;

            Send(type, &data);
        }

        void mEventManager::Send(const eEventType& type, sEventData* const data)
        {
            for (auto& event : _listeners[type])
                event.Invoke(data);
        }
    }
}