// engine.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "types.hpp"

namespace harpy
{
	class cContext;
	struct sApplicationCapabilities;
	class iApplication;

	class cEngine : public iObject
	{
		HARPY_OBJECT(cEngine)

	public:
		explicit cEngine(cContext* context, iApplication* app);
		virtual ~cEngine() override final = default;

		void Initialize();
		void Run();

		inline iApplication* GetApplication() const { return _app; }

	private:
		iApplication* _app = nullptr;
	};
}