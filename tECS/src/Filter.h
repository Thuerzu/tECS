#pragma once

#include "Types.h"
#include <array>
#include <algorithm>
#include <functional>
#include <tuple>


namespace tECS
{
	template <typename, typename>
	struct Selection;

	template <typename... IncludeTypes, typename... ExcludeTypes>
	struct Selection <TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>
	{	
	public:

		using IncludePack = TypePack<IncludeTypes...>;

		struct Iterator
		{
		public:
			static struct Begin {} begin;
			static struct End {} end;

		public:
			using ValueType = Entity;
			using PointerType = ValueType*;
			using RefType = ValueType&;

			Iterator(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> incl, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> excl, Begin)
				: Index(0), Types(incl), Exclude(excl)
			{
				if (!IsElementValid())
					operator++();
			}

			Iterator(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> incl, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> excl, End)
				: Types(incl), Exclude(excl)
			{
				Index = Types[0]->GetCount();
			}

		private:
			bool IsIndexValid()
			{
				return (Index >= 0 && Index < Types[0]->GetCount() );
			}

			bool Contains(Entity e)
			{
				if (std::any_of(Exclude.begin(), Exclude.end(),
					[&](IComponentStorageBase* excl)
					{ return excl->HasEntity(e); }))
					return false;

				if (std::all_of(Types.begin(), Types.end(),
					[&](IComponentStorageBase* incl)
					{ return incl->HasEntity(e); }))
					return true;

				return false;
			}

			bool IsElementValid()
			{
				return Contains(Types[0]->GetEntityAtIndex(Index));
			}

			template <typename T, typename Tuple>
			T&& GetArgument(Tuple&& t)
			{
				return std::get<T>(std::forward<Tuple>(t));
			}



		public:
			Iterator& operator ++()
			{
				do { ++Index; }
				while (IsIndexValid() && !IsElementValid());
				return *this;
			}

			Iterator& operator --()
			{
				do { --Index; }
				while (IsIndexValid() && !IsElementValid());
				return *this;
			}

			ValueType operator *() const
			{
				return Types[0]->GetEntityAtIndex(Index);
			}

			bool operator !=(const Iterator& o) const
			{
				return Index != o.Index;
			}

		private:
			size_t Index;
			std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> Types;
			std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> Exclude;
		};

	public:
		Selection(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> incl, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> excl)
			: Types(incl), Exclude(excl)
		{
			std::sort(Types.begin(), Types.end(), [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->GetCount() < b->GetCount(); });
			std::sort(Exclude.begin(), Exclude.end(), [](IComponentStorageBase* a, IComponentStorageBase* b) { return a->GetCount() > b->GetCount(); });
		}

		Selection(Selection&& other)
		{
			Types = std::move(other.Types);
			Exclude = std::move(other.Exclude);
		}

		~Selection() {}
		
	public:

		template <typename Func>
		void ForEach(Func&& fn)
		{
			for (auto e : *this)
			{
				Invoke(e, std::forward<Func>(fn), std::forward<IncludeTypes>(GetEntityComponent<IncludeTypes>(e))...);
			}
		}

		bool Contains(Entity e)
		{
			if (std::any_of(Exclude.begin(), Exclude.end(),
				[&](IComponentStorageBase* excl)
				{ return excl->HasEntity(e); }))
				return false;

			if (std::all_of(Types.begin(), Types.end(),
				[&](IComponentStorageBase* incl)
				{ return incl->HasEntity(e); }))
				return true;
			
			return false;
		}

	private:
		template <typename T>
		T& GetEntityComponent(Entity e)
		{
			for (size_t i = 0; i < sizeof...(IncludeTypes); ++i)
			{
				if (std::type_index(typeid(T)) == Types[i]->GetTypeIndex())
					return *dynamic_cast<ComponentStorage<T>*>(Types[i])->GetComponent(e);
			}
		}

		template <typename Func, typename... ArgTypes>
		void Invoke(Entity e, Func&& fn, ArgTypes&... incl)
		{
			std::tuple<Entity, ArgTypes&...> args{ e, std::forward<ArgTypes>(incl)... };
			using Traits = FunctionTraits<std::remove_reference_t<Func>>;
			using ParamPack = typename Traits::ArgTypes;
			InvokeDispatch(std::forward<Func>(fn), args, ParamPack{});
		}

		template <typename Func, typename ArgsTuple, typename... NeededArgs>
		void InvokeDispatch(Func&& fn, ArgsTuple& argsTuple, TypePack<NeededArgs...>)
		{
			std::invoke(std::forward<Func>(fn), GetMatchingType<NeededArgs>(argsTuple)...);
		}

		template <typename T, typename Tuple>
		T& GetMatchingType(Tuple&& tuple)
		{
			return std::get<T>(std::forward<Tuple>(tuple));
		}


	public:
		Iterator begin()
		{
			return Iterator(Types, Exclude, Iterator::begin);
		}

		Iterator end()
		{
			return Iterator(Types, Exclude, Iterator::end);
		}

	private:
		std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> Types;
		std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> Exclude;
	};

	template <typename... IncludeTypes, typename... ExcludeTypes>
	typename Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>::Iterator::Begin Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>::Iterator::begin;

	template <typename... IncludeTypes, typename... ExcludeTypes>
	typename Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>::Iterator::End Selection<TypePack<IncludeTypes...>, TypePack<ExcludeTypes...>>::Iterator::end;
}