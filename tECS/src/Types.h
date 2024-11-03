#pragma once

#include "Core.h"
#include <inttypes.h>
#include <typeindex>
#include <unordered_map>

namespace tECS
{
	struct TECS_API IBaseComponentStorage;

	using Entity = uint64_t;			//This is used as an ID; 0 is empty, starts at 1

	struct TECS_API IComponent
	{
		virtual ~IComponent() = default;
	};

	struct TECS_API ISystem
	{
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual ~ISystem() = default;
	};

	using ComponentRegistry = std::unordered_map<std::type_index, IBaseComponentStorage*>;
}