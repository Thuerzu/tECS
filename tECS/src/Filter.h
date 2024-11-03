#pragma once

#include "Core.h"
#include "Types.h"
#include <vector>
#include <algorithm>


namespace tECS
{
	struct TECS_API ComponentInfo
	{
		std::type_index Type;
		size_t InstanceCount;

		ComponentInfo(const std::type_index& ti, size_t ic)
			: Type(ti), InstanceCount(ic) {}

		bool operator < (const ComponentInfo& o) const
		{
			return InstanceCount < o.InstanceCount;
		}
	};

	struct TECS_API Filter
	{	
	public:
		struct TECS_API Iterator
		{
		public:
			static struct Begin {} begin;
			static struct End {} end;

		public:
			using ValueType = Entity;
			using PointerType = ValueType*;
			using RefType = ValueType&;

			Iterator(Filter* container, Begin);

			Iterator(Filter* container, End);

			Iterator& operator ++();

			Iterator& operator --();

			ValueType operator *() const;

			bool operator !=(const Iterator& o) const;

		private:
			bool IsIteratorValid() const;
			bool IsIndexValid() const;

		private:
			size_t Index;
			Filter* Container;
		};

	public:
		Filter(const ComponentRegistry* registry);

		Filter(Filter&& other);

		~Filter() {}
		
	public:
		template <typename... FilterTypes>
		void Select()
		{
			(UnpackTypeInfo<FilterTypes>(), ...);
			std::sort(Types.begin(), Types.end());
		}

		bool Contains(Entity e);

	public:
		Iterator begin();

		Iterator end();

	private:
		template <typename T>
		void UnpackTypeInfo(void)
		{
			Types.emplace_back(std::type_index(typeid(T)), Registry->at(std::type_index(typeid(T)))->GetCount());
		}

	private:
		const ComponentRegistry* Registry;
		std::vector<ComponentInfo> Types; 
	};
}