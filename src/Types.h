#pragma once

#include <inttypes.h>
#include <typeindex>
#include <unordered_map>
#include <tuple>
#include <utility>

namespace tECS {
	struct IComponentStorageBase;

	using Entity = uint64_t;			//This is used as an ID; 0 is empty, starts at 1

	using ComponentRegistry = std::unordered_map<std::type_index, IComponentStorageBase*>;

	template <size_t Idx, typename... T>
	struct PackElement;

	template <typename First, typename... Rest>
	struct PackElement<0, First, Rest...> {
		using Type = First;
	};

	template <size_t Index, typename First, typename... Rest>
	struct PackElement<Index, First, Rest...> {
		using Type = PackElement<Index - 1, Rest...>::Type;
	};

	template<size_t Idx>
	struct PackElement<Idx> {
		static_assert(false, "PackElement index out of bounds");
		using Type = void;
	}; // to trigger static_assert on out-of-bounds

	template <typename... T>
	struct TypePack
	{
		using Tuple = std::tuple<T...>;
		template <size_t N>
		using At = typename PackElement<N, T...>::Type;
		template <typename U>
		static constexpr bool Contains() {
			return (std::is_same_v<U, T> || ...);
		}
		constexpr size_t Size() const {
			return sizeof...(T);
		}
	};

	template <typename... T>
	using Exists = TypePack<T...>;

	template <typename... T>
	using Exclude = TypePack<T...>;

	template <size_t Idx, typename... T>
	struct PackElement<Idx, TypePack<T...>> {
		using Type = typename PackElement<Idx, T...>::Type;
	};

	template<size_t... I>
	struct Sequence;
	template <size_t First, size_t... Rest>
	struct Sequence<First, Rest...>{
		template<size_t... L>
		using Concat = Sequence<First, Rest..., L...>;
		constexpr size_t first() {
			return First;
		}
	};

	template <int64_t... I>
	struct Range;

	template <int64_t E>
	struct Range<E> {
		using Seq = Range<0, E>::Seq;
	};

	template <int64_t B, int64_t E>
	struct Range<B, E>{
		using Seq = Range<B, E, 1>::Seq;
	};

	template <int64_t B, int64_t E, int64_t S>
	struct Range<B, E, S> {
	private:
		constexpr static int64_t EndEx = E - 1;
		constexpr static int64_t Gap = (EndEx - B) % S;
		constexpr static int64_t ClipEnd = EndEx - Gap;
	public:
		using Seq = Range<B, ClipEnd, S, ClipEnd - B>::Seq;
	};

	template <int64_t B, int64_t E, int64_t S, int64_t R, int64_t... I>
	struct Range<B, E, S, R, I...> {
		using Seq = Range<B, E, S, R - S, B + R, I...>::Seq; 
	};

	template <int64_t B, int64_t E, int64_t S, int64_t... I>
	struct Range<B, E, S, 0, I...> {
		using Seq = Sequence<B, I...>;
	};

	template <typename T>
	struct FunctionTraits : FunctionTraits<decltype(&T::operator())> {};

	template <typename R, typename T, typename... Args>
	struct FunctionTraits<R(T::*)(Args...) const>
	{
		using ArgTypes = TypePack<Args...>;
		using ReturnType = R;
	};

	template <typename... T>
	struct TupleImpl;

	template<typename First, typename... Rest>
	struct TupleImpl<First, Rest...> : public TupleImpl<Rest...>{
		using RestTup = TupleImpl<Rest...>;
		template <size_t N>
		using Type = PackElement<N, First, Rest...>::Type;

		template<typename UFirst, typename... URest>
		constexpr TupleImpl(UFirst&& t, URest&&... r)
			: RestTup(std::forward<URest>(r)...), item(std::forward<UFirst>(t)) {}

		template<size_t Idx>
		inline Type<Idx>& get() {
		    if constexpr (Idx == 0)
		        return item;
		    else
		        return static_cast<RestTup*>(this)->template get<Idx - 1>();
		}

		template <typename T>
		inline T& get() {
			if constexpr (std::is_same_v<T, First>)
				return item;
			else
				return static_cast<RestTup*>(this)->template get<T>();
		} 

		static consteval size_t size() {
			return RestTup::size() + 1;
		} 

		First item;
	};

	template<typename T>
	struct TupleImpl<T> {
		constexpr TupleImpl(T&& t) : item(t) {}

		template<size_t N>
		inline T& get() {
			if constexpr(N == 0)
				return item;
		}

		template <typename Ty>
		inline auto& get() {
			if constexpr (std::is_same_v<T, Ty>)
				return item;
			else
				static_assert(false, "Type not found in Tuple");
		}

		static consteval size_t size() {
			return 1;
		}
		
		T item;
	};

	template <typename... Types>
	struct Tuple : public TupleImpl<Types...> {
		template<typename... U>
		constexpr Tuple(U&&... args) : TupleImpl<Types...>(std::forward<U>(args)...) {}
	};
}