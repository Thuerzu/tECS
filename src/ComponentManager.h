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
		Entity create_entity()
		{
			THLIB_BENCHMARK_FUNCTION;
			if (deleted_entities.empty())
				return ++next;
			Entity temp = deleted_entities.back();
			deleted_entities.pop_back();
			return temp;
		}

		void delete_entity(Entity e)
		{
			if (std::find(deleted_entities.begin(), deleted_entities.end(), e) != deleted_entities.end())
				return;
			for (auto comp_storage : components_data)
				comp_storage.second->remove_entity(e);
			deleted_entities.push_back(e);
		}

		//add the component
		template <typename Component>
		inline void add(Entity e, const Component& comp)
		{
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->add_entity(e, comp);
		}
		
		//emplace the component with the given arguments
		template <typename Component, typename... Args>
		inline void emplace(Entity e, Args&&... args)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->emplace_entity(e, std::forward<Args>(args)...);
		}

		//add or modify the component
		template <typename Component>
		inline void add_or_modify(Entity e, const Component& comp)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->add_or_modify_entity(e, comp);
		}

		//emplace or modify the component with the given arguments
		template <typename Component, typename... Args>
		inline void emplace_or_modify(Entity e, Args&&... args)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->emplace_or_modify_entity(e, std::forward<Args>(args)...);
		}

		//returns true if the entity has all the components
		template <typename... Component>
		inline bool has(Entity e)
		{
			(initialise_component_storage_if_empty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(components_data[std::type_index(typeid(Component))])->has_entity(e) && ...);
		}

		//returns true if the entity has any of the components
		template <typename... Component>
		inline bool has_any(Entity e)
		{
			(initialise_component_storage_if_empty<Component>(), ...);
			return (dynamic_cast<StorageTypePointer<Component>>(components_data[std::type_index(typeid(Component))])->has_entity(e) || ...);
		}

		//returns a pointer to the component, nullptr if it does not exist
		template <typename Component>
		inline Component* get(Entity e)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			return dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->get_component(e);
		}

		//returns a default value if the component does not exist, otherwise returns the component
		template <typename Component>
		inline Component get_or(Entity e, Component&& defaultValue)
		{
			THLIB_BENCHMARK_FUNCTION;
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			if (has<Component>(e))
				return *dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->get_component(e);
			return defaultValue;
		}

		//returns a pointer to the component, if it does not exist, it adds the default value
		template <typename Component>
		inline Component* get_or_add(Entity e, Component& defaultValue)
		{
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			if (!has<Component>(e))
				add(e, defaultValue);
			return dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->get_component(e);
		}

		//returns a pointer to the component, if it does not exist, it emplaces the component with the given arguments
		template <typename Component, typename... Args>
		inline Component* get_or_emplace(Entity e, Args... args)
		{
			static std::type_index type_idx = std::type_index(typeid(Component));
			initialise_component_storage_if_empty<Component>();
			if (!has<Component>(e))
				emplace<Component>(e, std::forward<Args>(args)...);
			return dynamic_cast<StorageTypePointer<Component>>(components_data[type_idx])->get_component(e);
		}

		//returns a selection of tuples that have all the components of IncludeTypes
		template <typename... IncludeTypes>
		TupleSelection<TypePack<IncludeTypes...>, TypePack<>> where()
		{
			THLIB_BENCHMARK_FUNCTION;
			return TupleSelection<TypePack<IncludeTypes...>, TypePack<>>{ std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ get_component_storage<IncludeTypes>()... }, std::array<IComponentStorageBase*, 0>{} };
		}

		//returns a selection of tuples that have all the components of IncludeTypes and none of the components of ExcludeTypes
		template <typename... IncludeTypes, typename... ExcludeTypes>
		TupleSelection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>> where(Exclude<ExcludeTypes...>)
		{
			THLIB_BENCHMARK_FUNCTION;
			return TupleSelection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>{ std::array<IComponentStorageBase*, sizeof...(IncludeTypes)>{ get_component_storage<IncludeTypes>()... }, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)>{ get_component_storage<ExcludeTypes>()... } };
		}

	private:
		template <typename Component>
		void initialise_component_storage_if_empty() {
			static std::type_index type_idx = std::type_index(typeid(Component));
			if (components_data[type_idx] == nullptr)
				components_data[type_idx] = new ComponentStorage<Component>;
		}

		template <typename Component>
		IComponentStorageBase* get_component_storage() {
			return components_data[std::type_index(typeid(Component))];
		}

		Entity next = 0;
		std::vector<Entity> deleted_entities = {};
		ComponentRegistry components_data = {};
	};
}
