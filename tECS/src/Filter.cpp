#include "Filter.h"
#include "ComponentStorage.h"

namespace tECS
{
	Filter::Iterator::Iterator(Filter* container, Begin)
		: Container(container)
	{
		Index = 0;
		if (!IsIteratorValid())
		{
			++(*this);
		}
	}

	Filter::Iterator::Iterator(Filter* container, End)
		: Container(container)
	{
		Index = Container->Types[0].InstanceCount;
	}

	Filter::Iterator& Filter::Iterator::operator++()
	{
		do 
		{ 
			++Index;
			if (IsIndexValid()) continue;
			Index = Container->Types[0].InstanceCount;
			break;
		} while (!IsIteratorValid());
		return *this;
	}
	Filter::Iterator& Filter::Iterator::operator--()
	{
		do { --Index; } while (!IsIteratorValid() && IsIndexValid());
		return *this;
	}
	Filter::Iterator::ValueType Filter::Iterator::operator*() const
	{
		return Container->Registry->at(Container->Types[0].Type)->GetEntityAtIndex(Index);
	}
	bool Filter::Iterator::operator!=(const Iterator& o) const
	{
		return !(Container == o.Container && Index == o.Index);
	}
	inline bool Filter::Iterator::IsIteratorValid() const
	{
		return Container->Contains(Container->Registry->at(Container->Types[0].Type)->GetEntityAtIndex(Index));
	}
	inline bool Filter::Iterator::IsIndexValid() const
	{
		return 0 <= Index && Index < Container->Types[0].InstanceCount;
	}
	Filter::Filter(const ComponentRegistry* registry)
	{
		Registry = registry;
	}
	Filter::Filter(Filter&& other)
	{
		Registry = other.Registry;
		Types = std::move(other.Types);
	}
	bool Filter::Contains(Entity e)
	{
		bool contained = true;
		for (auto& [type, _] : Types)
		{
			if (Registry->at(type)->HasEntity(e)) continue;
			contained = false;
			break;
		}
		return contained;
	}
	Filter::Iterator Filter::begin()
	{
		return Filter::Iterator(this, Filter::Iterator::begin);
	}
	Filter::Iterator Filter::end()
	{
		return Filter::Iterator(this, Filter::Iterator::end);
	}

	Filter::Iterator::Begin Filter::Iterator::begin;
	Filter::Iterator::End Filter::Iterator::end;
}