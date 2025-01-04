#pragma once

#include "Types.h"
#include "ComponentStorage.h"
#include "Filter.h"
#include <vector>
#include <typeindex>

namespace tECS
{
	struct ECS
	{
		template <typename Component>
		using StorageTypePointer = ComponentStorage<Component>*;
		Entity CreateEntity()
		{
			if (DeletedEntities.empty())
				return ++Next;
			Entity temp = DeletedEntities.back();
			DeletedEntities.pop_back();
			return temp;
		}

		void DeleteEntity(Entity& e)
		{
			for (auto compStorage : ComponentsData)
				compStorage.second->RemoveEntity(e);
			DeletedEntities.push_back(e);
		}

		template <typename Component>
		inline void Add(Entity e, const Component& comp)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->AddEntity(e, comp);
		}

		template <typename Component, typename... Args>
		inline void Emplace(Entity e, Args&&... args)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->EmplaceEntity(e, std::forward<Args>(args)...);
		}

		template <typename Component>
		inline void AddOrModify(Entity e, const Component& comp)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->AddOrModifyEntity(e, comp);
		}

		template <typename Component, typename... Args>
		inline void EmplaceOrModify(Entity e, Args&&... args)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->EmplaceOrModifyEntity(e, std::forward<Args>(args)...);
		}

		template <typename... Component>
		inline bool Has(Entity e)
		{
			(InitialiseComponentStorageIfEmpty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(ComponentsData[std::type_index(typeid(Component))])->HasEntity(e) && ...);
		}

		template <typename... Component>
		inline bool HasAny(Entity e)
		{
			(InitialiseComponentStorageIfEmpty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(ComponentsData[std::type_index(typeid(Component))])->HasEntity(e) || ...);
		}

		template <typename Component>
		inline Component* Get(Entity e)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			return dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
		}

		template <typename Component>
		inline Component GetOr(Entity e, Component defaultValue)
		{
			static std::type_index typeIndex = std::type_index(typeid(Component));
			InitialiseComponentStorageIfEmpty<Component>();
			if (Has<Component>(e))
				return *dynamic_cast<StorageTypePointer<Component>>(ComponentsData[typeIndex])->GetComponent(e);
			return defaultValue;
		}


		template <typename... IncludeTypes>
		Selection<TypePack<IncludeTypes...>, TypePack<>>&& Where()
		{
			return Selection<TypePack<IncludeTypes...>, TypePack<>>(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ GetComponentStorage<IncludeTypes>()... }, std::array<IComponentStorageBase*, 0>{});
		}

		template <typename... IncludeTypes, typename... ExcludeTypes>
		Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>&& Where(TypePack<ExcludeTypes...>)
		{
			return Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ GetComponentStorage<IncludeTypes>()... }, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)>{ GetComponentStorage<ExcludeTypes>()... });
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
