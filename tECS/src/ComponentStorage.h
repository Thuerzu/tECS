#pragma once

#include "Core.h"
#include <utility>
#include <unordered_map>

namespace tECS
{
	struct IComponentStorageBase
	{
	public:
		virtual size_t GetIndexOfEntity(Entity e) const = 0;
		virtual Entity GetEntityAtIndex(size_t i) const = 0;
		virtual void RemoveEntity(Entity e) = 0;
		virtual size_t GetCount() const = 0;
		virtual bool HasEntity(Entity e) const = 0;
		virtual std::type_index GetTypeIndex() const = 0;
	};

	template <typename Component>
	struct ComponentStorage : public IComponentStorageBase
	{
	public:

		ComponentStorage() = default;
		bool HasEntity(Entity e) const
		{
			return (EntityToIndex.find(e) != EntityToIndex.end());
		}
		size_t GetIndexOfEntity(Entity e) const
		{
			if (EntityToIndex.find(e) != EntityToIndex.end())
				return EntityToIndex.at(e);
			return 0;
		}
		Entity GetEntityAtIndex(size_t i) const
		{
			return IndexToEntity[i];
		}
		void AddEntity(Entity e, const Component& comp)
		{
			Storage.push_back(comp);
			IndexToEntity.push_back(e);
			EntityToIndex[e] = Storage.size() - 1;
		}
		template <typename... Args>
		void EmplaceEntity(Entity e, Args&&... args)
		{
			Storage.emplace_back(Component{ std::forward<Args>(args)... });
			IndexToEntity.push_back(e);
			EntityToIndex[e] = Storage.size() - 1;
		}
		void RemoveEntity(Entity e)
		{
			if (Storage.empty())
				return;
			auto i = EntityToIndex.find(e);
			if (i != EntityToIndex.end())
			{
				size_t index = i->second;
				if (index != Storage.size() - 1)
				{
					EntityToIndex.at(IndexToEntity.back()) = index;
					Storage.at(index) = std::move(Storage.back());
					IndexToEntity[index] = IndexToEntity.back();
				}
				IndexToEntity.pop_back();
				Storage.pop_back();
				EntityToIndex.erase(e);
			}
		}
		Component* GetComponent(Entity e)
		{
			auto iter = EntityToIndex.find(e);
			if (iter == EntityToIndex.end())
				return nullptr;
			return Storage.data() + (*iter).second;
		}

		std::vector<Component>* GetStorage()
		{
			return &Storage;
		}

		size_t GetCount() const
		{
			return EntityToIndex.size();
		}

		std::type_index GetTypeIndex() const
		{
			return std::type_index(typeid(Component));
		}

	private:
		std::vector<Component> Storage = {};
		std::unordered_map<Entity, size_t> EntityToIndex = {};
		std::vector<Entity> IndexToEntity = {};
	};
}