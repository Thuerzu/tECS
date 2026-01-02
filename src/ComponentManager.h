#pragma once

#include "Types.h"
#include "ComponentStorage.h"
#include "Selection.h"
#include <vector>
#include <typeindex>

#include <Macros.hpp>

namespace tECS
{
	struct ECS
	{
		template <typename Component>
		using StorageTypePointer = ComponentStorage<Component>*;
		Entity CreateEntity()
		{
			THLIB_BENCHMARK_FUNCTION;
			if (DeletedEntities.empty())
				return ++Next;
			Entity temp = DeletedEntities.back();
			DeletedEntities.pop_back();
			return temp;
		}

		void DeleteEntity(Entity e)
		{
			if (std::find(DeletedEntities.begin(), DeletedEntities.end(), e) != DeletedEntities.end())
				return;
			for (auto compStorage : ComponentsData)
				compStorage.second->RemoveEntity(e);
			DeletedEntities.push_back(e);
		}

		//add the component
		template <typename Component>
		inline void Add(Entity e, const Component& comp)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->AddEntity(e, comp);
		}
		
		//emplace the component with the given arguments
		template <typename Component, typename... Args>
		inline void Emplace(Entity e, Args&&... args)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->EmplaceEntity(e, std::forward<Args>(args)...);
		}

		//add or modify the component
		template <typename Component>
		inline void AddOrModify(Entity e, const Component& comp)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->AddOrModifyEntity(e, comp);
		}

		//emplace or modify the component with the given arguments
		template <typename Component, typename... Args>
		inline void EmplaceOrModify(Entity e, Args&&... args)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->EmplaceOrModifyEntity(e, std::forward<Args>(args)...);
		}

		//returns true if the entity has all the components
		template <typename... Component>
		inline bool Has(Entity e)
		{
			(InitialiseComponentStorageIfEmpty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(ComponentsData[std::type_index(typeid(Component))])->HasEntity(e) && ...);
		}

		//returns true if the entity has any of the components
		template <typename... Component>
		inline bool HasAny(Entity e)
		{
			(InitialiseComponentStorageIfEmpty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(ComponentsData[std::type_index(typeid(Component))])->HasEntity(e) || ...);
		}

		//returns a pointer to the component, nullptr if it does not exist
		template <typename Component>
		inline Component* Get(Entity e)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			return dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
		}

		//returns a default value if the component does not exist, otherwise returns the component
		template <typename Component>
		inline Component GetOr(Entity e, Component&& defaultValue)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			if (Has<Component>(e))
				return *dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
			return defaultValue;
		}

		//returns a pointer to the component, if it does not exist, it adds the default value
		template <typename Component>
		inline Component* GetOrAdd(Entity e, Component& defaultValue)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			if (!Has<Component>(e))
				Add(e, defaultValue);
			return dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
		}

		//returns a pointer to the component, if it does not exist, it emplaces the component with the given arguments
		template <typename Component, typename... Args>
		inline Component* GetOrEmplace(Entity e, Args... args)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			if (!Has<Component>(e))
				Emplace<Component>(e, std::forward<Args>(args)...);
			return dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
		}

		//returns a selection of entities that have all the components of IncludeTypes
		template <typename... IncludeTypes>
		Selection<Exists<IncludeTypes...>, Exclude<>> Where()
		{
			THLIB_BENCHMARK_FUNCTION;
			return Selection<Exists<IncludeTypes...>, Exclude<>>(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ GetComponentStorage<IncludeTypes>()... }, std::array<IComponentStorageBase*, 0>{});
		}

		//returns a selection of entities that have all the components of IncludeTypes and none of the components of ExcludeTypes
		template <typename... IncludeTypes, typename... ExcludeTypes>
		Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>> Where(Exclude<ExcludeTypes...>)
		{
			THLIB_BENCHMARK_FUNCTION;
			return Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ GetComponentStorage<IncludeTypes>()... }, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)>{ GetComponentStorage<ExcludeTypes>()... });
		}

	private:
		template <typename Component>
		void InitialiseComponentStorageIfEmpty()
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			if (ComponentsData[typeIndex] == nullptr)
				ComponentsData[typeIndex] = new ComponentStorage<Component>;
		}

		template <typename Component>
		IComponentStorageBase* GetComponentStorage()
		{
			return ComponentsData[std::type_index(typeid(Component))];
		}

		Entity Next = 0;
		std::vector<Entity> DeletedEntities = {};
		ComponentRegistry ComponentsData = {};
	};
}
