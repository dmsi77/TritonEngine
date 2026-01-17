// engine.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "types.hpp"

namespace realware
{
	struct sEngineCapabilities
	{
		types::usize _memoryAlignment = 64;
		types::usize _maxPhysicsSceneCount = 16;
		types::usize _maxPhysicsMaterialCount = 256;
		types::usize _maxPhysicsActorCount = 8192;
		types::usize _maxPhysicsControllerCount = 8;
	};

	class cEngine : public iObject
	{
		REALWARE_CLASS(cEngine)

	public:
		explicit cEngine(cContext* context, const sEngineCapabilities* capabilities);
		virtual ~cEngine() = default;

		void Initialize();

		const sEngineCapabilities* GetCapabilities() const { return _capabilities; }

	private:
		const sEngineCapabilities* _capabilities = nullptr;
	};
}