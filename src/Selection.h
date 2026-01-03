#pragma once

#include "Types.h"
#include <array>
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>
#include <ranges>

namespace tECS
{
	template <typename Includes, typename Excludes, typename Value>
	struct SelectionIterator;

	template <typename... Includes, typename... Excludes, typename Value>
	struct SelectionIterator<TypePack<Includes...>, TypePack<Excludes...>, Value> {
		template <size_t N>
		using ComponentStorageBase = std::array<IComponentStorageBase*, N>;
		using ValueType = Value;
		using PointerT = std::unordered_map<Entity, uintptr_t>::iterator;
		using RefType = ValueType&;
		using Self = SelectionIterator<TypePack<Includes...>, TypePack<Excludes...>, Value>;

		static constexpr size_t Incl = sizeof...(Includes);
		static constexpr size_t Excl = sizeof...(Excludes);

		SelectionIterator(ComponentStorageBase<Incl> incl, ComponentStorageBase<Excl> excl, PointerT init_it)
			: pointer(init_it), include(incl), exclude(excl) {
			while (pointer != include[0]->get_entity_component()->end()) {
				if (is_element_valid())
					break;
				operator++();
			}
		}

	private:
		template <typename T>
		T& get_entity_component(Entity e) {
			auto it = std::find_if(include.begin(), include.end(), [&](IComponentStorageBase* comp_storage) {
				return comp_storage->get_type_index() == std::type_index(typeid(T));
			});
			if (it == include.end())
				throw std::runtime_error("Entity does not have a component of the requested type!");
			return *dynamic_cast<ComponentStorage<std::remove_reference_t<T>>*>(*it)->get_component(e);
		}	

		bool is_index_valid() {
			return (pointer != include[0]->get_entity_component()->begin() && pointer != include[0]->get_entity_component()->end() );
		}

		bool contains(Entity e) {
			if (std::any_of(exclude.begin(), exclude.end(),
				[&](IComponentStorageBase* excl)
				{ return excl->has_entity(e); }))
				return false;

			if (std::all_of(include.begin(), include.end(),
				[&](IComponentStorageBase* incl)
				{ return incl->has_entity(e); }))
				return true;

			return false;
		}

		bool is_element_valid() {
			return contains((*pointer).first);
		}

	public:
		Self& operator ++() {
			do ++pointer;
			while (is_index_valid() && !is_element_valid());
			return *this;
		}
		
		ValueType operator *() {
			auto e = (*pointer).first;
			if constexpr (std::is_same_v<ValueType, Entity>)
				return e;
			else
				return ValueType(e, get_entity_component<Includes>(e)...);
		}

		bool operator !=(const Self& o) const {
			return pointer != o.pointer;
		}

	private:
		PointerT pointer;
		ComponentStorageBase<Incl> include;
		ComponentStorageBase<Excl> exclude;
	};

	template <typename Incl, typename Excl, typename Value>
	struct Selection;

	template <typename... Includes, typename... Excludes, typename Value>
	class Selection<TypePack<Includes...>, TypePack<Excludes...>, Value> {	
	public:
		static constexpr size_t Incl = sizeof...(Includes);
		static constexpr size_t Excl = sizeof...(Excludes);

		using Iterator = std::conditional_t<std::is_same_v<Value, Entity>,
				SelectionIterator<TypePack<Includes...>, TypePack<Excludes...>, Entity>,								//if Value is Entity, return Entity iterator
				SelectionIterator<TypePack<Includes...>, TypePack<Excludes...>, std::tuple<Entity, Includes&...>>>;		//else return tuple iterator
		using Self = Selection<TypePack<Includes...>, TypePack<Excludes...>, Value>;

	public:
		Selection(std::array<IComponentStorageBase*, Incl> incl, std::array<IComponentStorageBase*, Excl> excl) {
			include = incl;
			exclude = excl;
			
			std::ranges::sort(include, [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->get_count() < b->get_count(); });
			std::ranges::sort(exclude, [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->get_count() > b->get_count(); });
		}

		Selection(Selection&& other)
		{
			include = other.include;
			exclude = other.exclude;
		}

		~Selection() {}
		
	public:
		template <typename Func, typename... Args>
		inline void for_each(Func&& fn, TypePack<Args...> = {}) {
			if constexpr (sizeof...(Args) == 0) {
				using Traits = FunctionTraits<std::remove_reference_t<Func>>;
				for_each(std::forward<Func>(fn), typename Traits::ArgTypes{});
			} else {
				THLIB_BENCHMARK_FUNCTION;
				for (auto entt = entities(); auto e : entt)
					invoke(fn, get_entity_component<Args>(e)...);
			}
		}

		bool contains(Entity e) {
			if (std::any_of(exclude.begin(), exclude.end(),
				[&](IComponentStorageBase* excl)
				{ return excl->has_entity(e); }))
				return false;
			if (std::all_of(include.begin(), include.end(),
				[&](IComponentStorageBase* incl)
				{ return incl->has_entity(e); }))
				return true;
			return false;
		}

		Selection<TypePack<Includes...>, TypePack<Excludes...>, Entity> entities() {
			return Selection<TypePack<Includes...>, TypePack<Excludes...>, Entity>(include, exclude);
		}

	private:
		template <typename T>
		std::conditional_t<std::is_same_v<T, Entity>, Entity, T&> get_entity_component(Entity e) {
			if constexpr (std::is_same_v<T, Entity>)
				return e;
			else
				for (size_t i = 0; i < Incl; ++i)
					if (std::type_index(typeid(T)) == include[i]->get_type_index())
						return *dynamic_cast<ComponentStorage<std::remove_reference_t<T>>*>(include[i])->get_component(e);
			throw std::runtime_error("Entity does not have a component of the requested type!");
		}

		template <typename Func, typename... ArgTypes>
		void invoke(Func&& fn, ArgTypes... incl) {
			std::tuple<ArgTypes...> args{ incl... };
			using Traits = FunctionTraits<std::remove_reference_t<Func>>;
			using ParamPack = TypePack<typename Traits::ArgTypes...>;
			std::apply(std::forward<Func>(fn), args);
		}

	public:
		Iterator begin() {
			return Iterator(include, exclude, include[0]->get_entity_component()->begin());
		}

		Iterator end() {
			return Iterator(include, exclude, include[0]->get_entity_component()->end());
		}

	private:
		std::array<IComponentStorageBase*, Incl> include;
		std::array<IComponentStorageBase*, Excl> exclude;
	};
}