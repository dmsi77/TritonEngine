// object.hpp

#pragma once

#include <string>
#include <typeinfo>
#include "log.hpp"
#include "types.hpp"

namespace realware
{
	class cContext;

	class cIdentifier
	{
	public:
		using ID = std::string;

		cIdentifier(const std::string& id);
		~cIdentifier() = default;

		inline const ID& GetID() const { return _id; }

	private:
		ID _id = "";
	};

	class cType
	{
	public:
		using Name = std::string;

		cType(const std::string& name) : _name(name) {}
		~cType() = default;

		template <typename T>
		static Name GetTypeNameDynamic() { return typeid(T).name(); }

		inline const Name& GetName() const { return _name; }

	private:
		Name _name = "";
	};

	class iObject
	{
	public:
		explicit iObject(cContext* context) : _context(context) {}
		virtual ~iObject() = default;

		virtual cType GetType() const = 0;
		inline cContext* GetContext() const { return _context; }

	protected:
		cContext* _context = nullptr;
	};

	class cFactoryObject : public iObject
	{
		friend class iFactory;

	public:
		explicit cFactoryObject(cContext* context) : iObject(context) {}
		cFactoryObject(const cFactoryObject& rhs) = delete;
		cFactoryObject& operator=(const cFactoryObject& rhs) = delete;
		virtual ~cFactoryObject() = default;

	protected:
		cIdentifier* _identifier = nullptr;
	};

	class iFactory : public iObject
	{
	public:
		explicit iFactory(cContext* context) : iObject(context) {}
		virtual ~iFactory() = default;

		virtual iObject* Create() = 0;

	protected:
		types::usize _counter = 0;
	};

	template <typename T>
	class ñFactory : public iFactory
	{
	public:
		explicit ñFactory(cContext* context) : iFactory(context) {}
		virtual ~ñFactory() = default;

		virtual cFactoryObject* Create() override final
		{
			if (_counter >= types::K_USIZE_MAX)
			{
				Print("Error: can't create object of type " + cType::GetTypeNameDynamic<T>() + "!");

				return nullptr;
			}

			T* object = new T(_context);
			const std::string id = object->GetType()->GetName() + std::to_string(_counter++);
			object._identifier = new cIdentifier(id);

			return (cFactoryObject*)object;
		}
	};
}