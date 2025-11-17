// event_manager.hpp

#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace game
    {
        class cGameObject;
    }

    namespace utils
    {
        enum class eEventType
        {
            NONE,
            KEY_PRESS
        };

        struct sEventData
        {
            types::usize ByteSize = 0;
            void* Data = nullptr;
        };

        using EventFunction = std::function<void(sEventData* const data)>;

        class cEvent
        {
            friend class mEventManager;

        public:
            cEvent(const eEventType& type, const EventFunction& function);
            ~cEvent() = default;

            void Invoke(sEventData* const data);
            inline game::cGameObject* GetReceiver() { return _receiver; }
            inline eEventType GetType() { return _type; }
            inline EventFunction& GetFunction() { return _function; }

        private:
            game::cGameObject* _receiver = nullptr;
            eEventType _type = eEventType::NONE;
            EventFunction _function;
        };

        class mEventManager
        {
        public:
            explicit mEventManager(const app::cApplication* const app);
            ~mEventManager() = default;
            
            void Subscribe(const game::cGameObject* receiver, cEvent& event);
            void Send(const eEventType& type);
            void Send(const eEventType& type, sEventData* const data);

        private:
            app::cApplication* _app = nullptr;
            std::unordered_map<eEventType, std::vector<cEvent>> _listeners;
        };
    }
}