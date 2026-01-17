// object.hpp

#pragma once

#include <string>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "types.hpp"

namespace realware
{
	class cContext;
	class cDataBuffer;
	class cGameObject;
	template <typename T>
	class cFactory;

	using ClassType = std::string;

	#define REALWARE_CLASS(typeName) \
		public: static ClassType GetType() { return #typeName; }

	class cIdentifier
	{
		REALWARE_CLASS(cIdentifier)

	public:
		using ID = std::string;

	public:
		cIdentifier(const std::string& id);
		~cIdentifier() = default;

		inline const ID& GetID() const { return _id; }

	private:
		ID _id = "";
	};

	class iObject
	{
		REALWARE_CLASS(iObject)

	public:
		explicit iObject(cContext* context) : _context(context) {}
		virtual ~iObject() = default;

		iObject(const iObject& rhs) = delete;
		iObject& operator=(const iObject& rhs) = delete;

		inline cContext* GetContext() const { return _context; }

	protected:
		cContext* _context = nullptr;
	};

	class cFactoryObject : public iObject
	{
		REALWARE_CLASS(cFactoryObject)

		template <typename T>
		friend class cFactory;
		friend class cMemoryAllocator;

	public:
		explicit cFactoryObject(cContext* context) : iObject(context) {}
		virtual ~cFactoryObject() override;

		template <typename... Args>
		void Subscribe(const std::string& id, eEventType type, Args&&... args);
		void Unsubscribe(eEventType type, cGameObject* receiver);
		void Send(eEventType type);
		void Send(eEventType type, cDataBuffer* data);

		inline cIdentifier* GetIdentifier() const { return _identifier; }

	protected:
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		cIdentifier* _identifier = nullptr;
	};

	template <typename T>
	class cFactory : public iObject
	{
		REALWARE_CLASS(ÒFactory)

	public:
		explicit cFactory(cContext* context) : iObject(context) {}
		virtual ~cFactory() = default;

		template <typename... Args>
		T* Create(Args&&... args);
		void Destroy(T* object);

	private:
		types::usize _counter = 0;
	};

	template <typename... Args>
	void cFactoryObject::Subscribe(const std::string& id, eEventType type, Args&&... args)
	{
		const cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
		dispatcher->Subscribe<Args>(id, type, std::forward<Args>(args)...);
	}

	template <typename T>
	template <typename... Args>
	T* cFactory<T>::Create(Args&&... args)
	{
		if (_counter >= types::K_USIZE_MAX)
		{
			Print("Error: can't create object of type '" + T::GetType() + "'!");

			return nullptr;
		}
		
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		const types::usize objectByteSize = sizeof(T);
		T* object = (T*)memoryAllocator->Allocate(objectByteSize, 64);
		new (object) T(std::forward<Args>(args)...);
		
		const std::string id = object->GetType() + std::to_string(_counter++);
		object->_identifier = new cIdentifier(id);

		return object;
	}

	template <typename T>
	void cFactory<T>::Destroy(T* object)
	{
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		object->~T();
		memoryAllocator->Deallocate(object);
	}
}