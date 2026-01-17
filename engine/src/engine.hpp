// engine.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "types.hpp"

namespace realware
{
	class iApplication;

	struct sEngineCapabilities
	{
		std::string windowTitle = "Test app";
		types::usize windowWidth = 640;
		types::usize windowHeight = 480;
		types::usize memoryAlignment = 64;
		types::usize maxPhysicsSceneCount = 16;
		types::usize maxPhysicsMaterialCount = 256;
		types::usize maxPhysicsActorCount = 8192;
		types::usize maxPhysicsControllerCount = 8;
	};

	class cEngine : public cFactoryObject
	{
		REALWARE_CLASS(cEngine)

	public:
		explicit cEngine(cContext* context, const sEngineCapabilities* capabilities, iApplication* app);
		virtual ~cEngine() = default;

		void Initialize();
		void Run();

		const sEngineCapabilities* GetCapabilities() const { return _capabilities; }

	private:
		const sEngineCapabilities* _capabilities = nullptr;
		iApplication* _app = nullptr;
	};
}