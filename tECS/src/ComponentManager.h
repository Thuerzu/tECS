#pragma once

#include "Core.h"
#include "Types.h"
#include "ComponentStorage.h"
#include "Filter.h"
#include <vector>
#include <typeindex>

namespace tECS
{
	struct TECS_API ECS
	{
		Entity CreateEntity();

		void DeleteEntity(Entity& e);

		template <typename Component>
		inline void Add(Entity e, const Component& comp)
		{
			using StorageTypePointer = ComponentStorage<Component>*;
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			((StorageTypePointer)ComponentsData[typeIndex])->AddEntity(e, comp);
		}

		template <typename Component, typename... Args>
		inline void Emplace(Entity e, Args&&... args)
		{
			using StorageTypePointer = ComponentStorage<Component>*;
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			((StorageTypePointer)ComponentsData[typeIndex])->EmplaceEntity(e, std::forward<Args>(args)...);
		}

		template <typename Component>
		inline Component* Get(Entity e)
		{
			using StorageTypePointer = ComponentStorage<Component>*;
			InitialiseComponentStorageIfEmpty<Component>();
			return ((StorageTypePointer)ComponentsData[std::type_index(typeid(Component))])->GetComponent(e);
		}

		template <typename... FilterTypes>
		Filter View()
		{
			auto filter = Filter(&ComponentsData);
			filter.Select<FilterTypes...>();
			return filter;
		}

		void RegisterSystem(ISystem* sys);

		void UpdateSystems(double deltaTime);

		inline double GetDeltaTime();

	private:
		template <typename Component>
		void InitialiseComponentStorageIfEmpty()
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			if (ComponentsData[typeIndex] == nullptr)
				ComponentsData[typeIndex] = new ComponentStorage<Component>;
		}

		Entity Next = 0;
		double DeltaTime;
		std::vector<Entity> DeletedEntities = {};
		ComponentRegistry ComponentsData = {};
		std::vector<ISystem*> Systems = {};
	};
}
