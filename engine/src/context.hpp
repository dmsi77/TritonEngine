// context.hpp

#pragma once

#include <unordered_map>
#include "types.hpp"

namespace realware
{
	class cObject;

	class cContext
	{
	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T>
		void Create();
	};
}