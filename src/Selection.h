#pragma once

#include "Types.h"
#include <array>
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>


namespace tECS
{
	template <typename, typename>
	struct Selection;

	template <typename... IncludeTypes, typename... ExcludeTypes>
	struct Selection <Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>
	{	
	public:

		using IncludePack = Exists<IncludeTypes...>;
		using PointerT = std::unordered_map<Entity, uintptr_t>::iterator;
		template <size_t N>
		using ComponentStorageBase = std::array<IComponentStorageBase*, N>;

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
				: Pointer(0), Types(incl), Exclude(excl)
			{
				Pointer = Types[0]->GetEntityComponent()->begin();
				while (!IsElementValid())
					operator++();
			}

			Iterator(std::array<IComponentStorageBase*, sizeof...(IncludeTypes)> incl, std::array<IComponentStorageBase*, sizeof...(ExcludeTypes)> excl, End)
				: Types(incl), Exclude(excl)
			{
				Pointer = Types[0]->GetEntityComponent()->end();
			}

		private:
			bool IsIndexValid()
			{
				return (Pointer != Types[0]->GetEntityComponent()->begin() && Pointer != Types[0]->GetEntityComponent()->end() );
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
				return Contains((*Pointer).first);
			}

			template <typename T, typename Tuple>
			T&& GetArgument(Tuple&& t)
			{
				return std::get<T>(std::forward<Tuple>(t));
			}



		public:
			Iterator& operator ++()
			{
				do { ++Pointer; }
				while (IsIndexValid() && !IsElementValid());
				return *this;
			}
			
			ValueType operator *() const
			{
				return (*Pointer).first;
			}

			bool operator !=(const Iterator& o) const
			{
				return Pointer != o.Pointer;
			}

		private:
			PointerT Pointer;
			ComponentStorageBase<sizeof...(IncludeTypes)> Types;
			ComponentStorageBase<sizeof...(ExcludeTypes)> Exclude;
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
			Types = other.Types;
			Exclude = other.Exclude;
		}

		~Selection() {}
		
	public:
		template <typename Func, typename... Args>
		inline void ForEach(Func&& fn, TypePack<Args...> = {}) {
			if constexpr (sizeof...(Args) == 0)
			{
				using Traits = FunctionTraits<std::remove_reference_t<Func>>;
				ForEach(std::forward<Func>(fn), typename Traits::ArgTypes{});
			} else {
				THLIB_BENCHMARK_FUNCTION;
				for (auto e : *this)
					Invoke(fn, GetEntityComponent<Args>(e)...);
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
		std::conditional_t<std::is_same_v<T, Entity>, Entity, T&> GetEntityComponent(Entity e)
		{
			if constexpr (std::is_same_v<T, Entity>) {
				return e;
			}
			else 
				for (size_t i = 0; i < sizeof...(IncludeTypes); ++i) {
					if (std::type_index(typeid(T)) == Types[i]->GetTypeIndex())
						return *dynamic_cast<ComponentStorage<std::remove_reference_t<T>>*>(Types[i])->GetComponent(e);
				}
		}

		template <typename Func, typename... ArgTypes>
		void Invoke(Func&& fn, ArgTypes... incl)
		{
			std::tuple<ArgTypes...> args{ incl... };
			using Traits = FunctionTraits<std::remove_reference_t<Func>>;
			using ParamPack = TypePack<typename Traits::ArgTypes...>;
			std::apply(std::forward<Func>(fn), args);//InvokeDispatch(std::forward<Func>(fn), args, ParamPack{});
		}

		template <typename Func, typename ArgsTuple, typename... NeededArgs>
		void InvokeDispatch(Func&& fn, ArgsTuple& argsTuple, TypePack<NeededArgs...>)
		{
			using Traits = FunctionTraits<std::remove_reference_t<Func>>;
			using FuncArgTuple = typename Traits::ArgTypes::Tuple;
			constexpr size_t funcN = std::tuple_size_v<FuncArgTuple>;
			constexpr size_t total = std::tuple_size_v<std::remove_reference_t<ArgsTuple>>;

			if constexpr (funcN == total)
			{
				std::apply(std::forward<Func>(fn), argsTuple);
			}
			else
			{
				// assume first element of argsTuple is Entity — forward the rest
				auto tuple_skip_first = [&]<size_t... I>(std::index_sequence<I...>)
				{
					return std::make_tuple(std::get<I + 1>(argsTuple)...);
				};

				auto sub = tuple_skip_first(std::make_index_sequence<(total > 0 ? total - 1 : 0)>{});
				std::apply(std::forward<Func>(fn), sub);
			}
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
	typename Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>::Iterator::Begin Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>::Iterator::begin;

	template <typename... IncludeTypes, typename... ExcludeTypes>
	typename Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>::Iterator::End Selection<Exists<IncludeTypes...>, Exclude<ExcludeTypes...>>::Iterator::end;
}