#pragma once

#include <inttypes.h>
#include <typeindex>
#include <unordered_map>
#include <tuple>

namespace tECS
{
	struct IComponentStorageBase;

	using Entity = uint64_t;			//This is used as an ID; 0 is empty, starts at 1

	using ComponentRegistry = std::unordered_map<std::type_index, IComponentStorageBase*>;

	template <typename... T>
	struct TypePack
	{
		using Tuple = std::tuple<T...>;
		constexpr size_t Size()
		{
			constexpr return sizeof...(T);
		}
	};

	template <typename... T>
	using Exists = TypePack<T...>;

	template <typename... T>
	using Exclude = TypePack<T...>;

	template <size_t, typename>
	struct PackElement;

	template <typename First, typename... Rest>
	struct PackElement<0, TypePack<First, Rest...>>
	{
		using Type = First;
	};

	template <size_t Index, typename First, typename... Rest>
	struct PackElement<Index, TypePack<First, Rest...>>
	{
		using Type = PackElement<Index - 1, Rest...>;
	};

	template <typename T>
	struct FunctionTraits : FunctionTraits<decltype(&T::operator())> {};

	template <typename R, typename T, typename... Args>
	struct FunctionTraits<R(T::*)(Args...) const>
	{
		using ArgTypes = TypePack<Args...>;
		using ReturnType = R;
	};

}