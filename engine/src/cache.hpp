// cache.hpp

#pragma once

#include "hash_table.hpp"
#include "context.hpp"
#include "types.hpp"

namespace triton
{
	template <typename T>
	class cCacheObject : public cObjectPtr {};

	template <typename T>
	class cCache : public iObject
	{
		TRITON_OBJECT(cCache)

	public:
		explicit cCache(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc);
		virtual ~cCache() override final = default;

		template <typename... Args>
		cCacheObject<T> Create(Args&&... args);
		cCacheObject<T> Find(const cTag& idStr);
		void Destroy(const cTag& idStr);

		inline T* GetElement(types::u32 index) const { return _objects->GetElement(index); }
		inline types::usize GetElementCount() const { return _objects->GetElementCount(); }

	private:
		cMap<T>* _objects = nullptr;
	};

	template <typename T>
	cCache<T>::cCache(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc) : iObject(context)
	{
		_objects = _context->Create<cMap<T>>(_context, allocatorDesc);
	}

	template <typename T>
	template <typename... Args>
	cCacheObject<T> cCache<T>::Create(Args&&... args)
	{
		cCacheObject<T> co = {};
		co.object = _objects->Insert(std::forward<Args>(args)...);

		return co;
	}

	template <typename T>
	cCacheObject<T> cCache<T>::Find(const cTag& idStr)
	{
		return _objects->Find(id);
	}

	template <typename T>
	void cCache<T>::Destroy(const cTag& idStr)
	{
		_objects->Erase(id);
	}
}