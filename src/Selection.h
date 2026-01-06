#pragma once

#include "Types.h"
#include <array>
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>
#include <ranges>

namespace tECS {
	template <size_t Incl, size_t Excl>
	struct BasicSelectionIterator {
		template <size_t N>
		using ComponentStorageBase = std::array<IComponentStorageBase*, N>;
		using PointerT = std::unordered_map<Entity, uintptr_t>::iterator;
		using Self = BasicSelectionIterator<Incl, Excl>;

		BasicSelectionIterator(ComponentStorageBase<Incl> incl, ComponentStorageBase<Excl> excl, PointerT init_it)
			: pointer(init_it), include(incl), exclude(excl) {
			while (pointer != include[0]->get_entity_component()->end()) {
				if (is_element_valid())
					break;
				operator++();
			}
		}

	private:
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

		bool operator !=(const Self& o) const {
			return pointer != o.pointer;
		}

	protected:
		PointerT pointer;
		ComponentStorageBase<Incl> include;
		ComponentStorageBase<Excl> exclude;
	};


	template <size_t Incl, size_t Excl>
	struct EntitySelectionIterator : public BasicSelectionIterator<Incl, Excl> {
		using BasicSelectionIterator<Incl, Excl>::BasicSelectionIterator;
		Entity operator *() {
			return (*this->pointer).first;
		}
	};

	template <typename Incl, typename Excl>
	struct TupleSelectionIterator;

	template <typename... Includes, typename... Excludes>
	struct TupleSelectionIterator<TypePack<Includes...>, TypePack<Excludes...>> : public BasicSelectionIterator<sizeof...(Includes), sizeof...(Excludes)> {
		using ValueType = std::tuple<Entity, Includes&...>;
		using BasicSelectionIterator<sizeof...(Includes), sizeof...(Excludes)>::BasicSelectionIterator;

		ValueType operator *() {
			auto e = (*this->pointer).first;
			return ValueType(e, get_entity_component<Includes>(e)...);
		}
	protected:
		template <typename T>
		T& get_entity_component(Entity e) {
			auto it = std::find_if(this->include.begin(), this->include.end(), [&](IComponentStorageBase* comp_storage) {
				return comp_storage->get_type_index() == std::type_index(typeid(T));
			});
			if (it == this->include.end())
				throw std::runtime_error("Entity does not have a component of the requested type!");
			return *dynamic_cast<ComponentStorage<std::remove_reference_t<T>>*>(*it)->get_component(e);
		}	
	};

	template <size_t Incl, size_t Excl>
	struct BasicSelection {	
	public:
		static constexpr size_t InclSize = Incl;
		static constexpr size_t ExclSize = Excl;
		
		BasicSelection(std::array<IComponentStorageBase*, InclSize> incl, std::array<IComponentStorageBase*, ExclSize> excl) {
			include = incl;
			exclude = excl;
			std::ranges::sort(include, [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->get_count() < b->get_count(); });
			std::ranges::sort(exclude, [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->get_count() > b->get_count(); });
		}

		BasicSelection(BasicSelection&& other)
		{
			include = other.include;
			exclude = other.exclude;
		}

		~BasicSelection() {}
		
	public:
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

	protected:
		std::array<IComponentStorageBase*, Incl> include;
		std::array<IComponentStorageBase*, Excl> exclude;
	};

	template <size_t Incl, size_t Excl>
	struct EntitySelection : public BasicSelection<Incl, Excl> {

		using BasicType = BasicSelection<Incl, Excl>;
		using BasicType::BasicSelection;
		using Iterator = EntitySelectionIterator<Incl, Excl>;

		template <typename RetType>
		inline void for_each(std::function<RetType(Entity)>&& fn) {
			THLIB_BENCHMARK_FUNCTION;
			for (auto e : *this)
				fn(e);
		}

		Iterator begin() {
			return Iterator(this->include, this->exclude, this->include[0]->get_entity_component()->begin());
		}

		Iterator end() {
			return Iterator(this->include, this->exclude, this->include[0]->get_entity_component()->end());
		}
	};

	template <typename Incl, typename Excl>
	struct TupleSelection;

	template <typename... Includes, typename... Excludes>
	struct TupleSelection< TypePack<Includes...>, TypePack<Excludes...> > : public BasicSelection<sizeof...(Includes), sizeof...(Excludes)> {
		using Iterator = TupleSelectionIterator<TypePack<Includes...>, TypePack<Excludes...>>;
		
		using BasicType = BasicSelection<sizeof...(Includes), sizeof...(Excludes)>;
		using BasicType::BasicType;

	protected:
		template <typename T>
		std::conditional_t<std::is_same_v<T, Entity>, Entity, T&> get_entity_component(Entity e) {
			if constexpr (std::is_same_v<T, Entity>)
				return e;
			else {
				auto it = std::find_if(this->include.begin(), this->include.end(), [&](IComponentStorageBase* comp_storage) {
					return comp_storage->get_type_index() == std::type_index(typeid(T));
				});
				if (it == this->include.end()) [[likely]]
					throw std::runtime_error("Entity does not have a component of the requested type!");
				return *dynamic_cast<ComponentStorage<std::remove_reference_t<T>>*>(*it)->get_component(e);
			}
		}

		template <typename Func, typename... ArgTypes>
		void invoke(Func&& fn, ArgTypes... incl) {
			std::tuple<ArgTypes...> args{ incl... };
			using Traits = FunctionTraits<std::remove_reference_t<Func>>;
			using ParamPack = TypePack<typename Traits::ArgTypes...>;
			std::apply(std::forward<Func>(fn), args);
		}

	public:
		EntitySelection<BasicType::InclSize, BasicType::ExclSize> entities() {
			return EntitySelection<BasicType::InclSize, BasicType::ExclSize>(this->include, this->exclude);
		}

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

		Iterator begin() {
			return Iterator(this->include, this->exclude, this->include[0]->get_entity_component()->begin());
		}

		Iterator end() {
			return Iterator(this->include, this->exclude, this->include[0]->get_entity_component()->end());
		}
	};
}