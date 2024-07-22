#pragma once

#include "Types.h"
#include "Core.h"
#include <vector>

namespace tECS
{
	struct TECS_API ECS
	{
		Entity CreateEntity();

		void DeleteEntity(Entity& e);

		template <typename Component>
		inline void AddComponent(Entity e, const Component& comp)
		{
			ComponentStorage<Component>::Get().AddEntity(e, comp);
		}

		template <typename Component>
		inline Component* GetComponent(Entity e)
		{
			return ComponentStorage<Component>::Get().GetComponent(e);
		}

		void RegisterSystem(ISystem* sys);

		void UpdateSystems(double deltaTime);

		inline double GetDeltaTime();

		static ECS* Get();

	private:
		Entity Next = 0;
		double DeltaTime;
		std::vector<Entity> DeletedEntities = {};
		std::vector<IBaseComponentStorage*> ComponentsData = {};
		std::vector<ISystem*> Systems = {};

	private:
		static ECS* s_Instance;
	};
}
