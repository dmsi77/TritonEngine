// object.cpp

#include "object.hpp"
#include "event_manager.hpp"
#include "context.hpp"

using namespace types;

namespace triton
{
    cIdentifier::cIdentifier(usize& counter, const std::string& typeName)
    {
        const std::string& strID = typeName + std::to_string(counter++);
        memcpy(&_id[0], &strID.c_str()[0], kMaxIDStringByteSize);
    }

    cIdentifier cIdentifier::Generate(const std::string& seed)
    {
        static usize counter = 0;

        return cIdentifier(counter, seed);
    }

    iObject::~iObject()
    {
        delete _identifier;
    }

    void iObject::Subscribe(eEventType type, EventFunction&& function)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Subscribe(this, type, std::move(function));
    }

    void iObject::Unsubscribe(eEventType type)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Unsubscribe(this, type);
    }

    void iObject::Send(eEventType type)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type);
    }

    void iObject::Send(eEventType type, cDataBuffer* data)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type, data);
    }
}