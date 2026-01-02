#pragma once

#include "Bitset.h"
#include "Types.h"

#include <utility>
#include <unordered_map>
#include <array>
#include <memory>
#include <cstdint>
#include <vector>
#include <variant>

#include <Macros.hpp>

namespace tECS
{
	struct IComponentStorageBase
	{
	public:
		virtual Entity GetEntityOfComponent(uintptr_t ptr) const = 0;
		virtual void RemoveEntity(Entity e) = 0;
		virtual size_t GetCount() const = 0;
		virtual bool HasEntity(Entity e) const = 0;
		virtual std::unordered_map<Entity, uintptr_t>* GetEntityComponent() = 0;
		virtual std::type_index GetTypeIndex() const = 0;
	};

	template <typename Component>
	struct ComponentBlock
	{
	public:
		ComponentBlock() = default;
		
		uintptr_t Add(Entity e, const Component& comp)
		{
			if (Full())
				return 0;
			size_t index = Tombstones.CountrOne();
			Storage[index] = comp;
			Tombstones.Set(index);
			Entities[index] = e;
			return reinterpret_cast<uintptr_t>(&Storage[index]);
		}

		template <typename... Args>
		uintptr_t Emplace(Entity e, Args&&... args)
		{
			if (Full())
				return 0;
			size_t index = Tombstones.CountrOne();
			Storage[index] = Component{ std::forward<Args>(args)... };
			Tombstones.Set(index);
			Entities[index] = e;
			return reinterpret_cast<uintptr_t>(&Storage[index]);
		}

		Component* Get(Entity e)
		{
			for (size_t i = 0; i < Storage.size(); i++)
			{
				if (Tombstones.Test(i) && Entities[i] == e)
					return &Storage[i];
			}
			return nullptr;
		}

		bool Remove(Entity e)
		{
			for (size_t i = 0; i < Storage.size(); i++)
			{
				if (Tombstones.Test(i) && Entities[i] == e)
				{
					Tombstones.Reset(i);
					return true;
				}
			}
			return false;
		}

		bool RemoveAt(uintptr_t ptr)
		{
			uintptr_t base = reinterpret_cast<uintptr_t>(&Storage[0]);

			if (ptr < base || ptr >= base + sizeof(Component) * Storage.size())
				return false;

			size_t index = (ptr - base) / sizeof(Component);
			Tombstones.Reset(index);
			return true;
		}

		Entity GetEntityOfComponent(uintptr_t ptr)
		{
			uintptr_t base = reinterpret_cast<uintptr_t>(&Storage[0]);

			if (ptr < base || ptr >= base + sizeof(Component) * Storage.size())
				return 0;

			size_t index = (ptr - base) / sizeof(Component);

			if (Tombstones.Test(index))
				return Entities[index];
			return 0;
		}

		bool Full() const
		{
			return Tombstones.All();
		}

		bool Empty() const
		{
			return Tombstones.None();
		}

	public:
		std::array<Component, 1024> Storage;
		std::array<Entity, 1024> Entities;
		Bitset<1024> Tombstones;
	};

	template <typename Component>
	struct ComponentStorage : public IComponentStorageBase
	{
	public:

		ComponentStorage() = default;
		bool HasEntity(Entity e) const
		{
			return (EntityComponent.find(e) != EntityComponent.end());
		}
		Entity GetEntityOfComponent(uintptr_t comp) const
		{
			for (const auto& pair : EntityComponent)
			{
				if (reinterpret_cast<uintptr_t>(pair.second) == comp)
					return pair.first;
			}
			return 0;
		}
		void AddEntity(Entity e, const Component& comp)
		{
			for (auto& block : Storage)
			{
				if (auto compPtr = block->Add(e, comp))
				{
					EntityComponent[e] = compPtr;
					return;
				}
			}
			//No space found, create new block
			auto newBlock = std::make_unique<ComponentBlock<Component>>();
			auto compPtr = newBlock->Add(e, comp);
			Storage.push_back(std::move(newBlock));
			EntityComponent[e] = compPtr;
		}
		void AddOrModifyEntity(Entity e, const Component& comp)
		{
			if (HasEntity(e))
			{
				*reinterpret_cast<Component*>(EntityComponent[e]) = comp;
				return;
			}
			AddEntity(e, comp);
		}

		template <typename... Args>
		void EmplaceEntity(Entity e, Args&&... args)
		{
			for (auto& block : Storage)
			{
				if (auto compPtr = block->Emplace(e, std::forward<Args>(args)...))
				{
					EntityComponent[e] = compPtr;
					return;
				}
			}
			//No space found, create new block
			THLIB_SET_MARKER("NEW BLOCK");
			auto newBlock = std::make_unique<ComponentBlock<Component>>();
			auto comp = newBlock->Emplace(e, std::forward<Args>(args)...);
			Storage.push_back(std::move(newBlock));
			EntityComponent[e] = comp;
		}

		template<typename... Args>
		void EmplaceOrModifyEntity(Entity e, Args&&... args)
		{
			if (HasEntity(e))
			{
				*reinterpret_cast<Component*>(EntityComponent[e]) = Component{ std::forward<Args>(args)... };
				return;
			}
			EmplaceEntity(e, std::forward<Args>(args)...);
		}

		void RemoveEntity(Entity e)
		{
			if (Storage.empty())
				return;

			auto i = EntityComponent.find(e);
			if (i != EntityComponent.end())
			{
				for (auto blockIt = Storage.begin(); blockIt < Storage.end(); blockIt++)
					if ((*blockIt)->RemoveAt(i->second))
					{
						if ((*blockIt)->Empty())
							blockIt = Storage.erase(blockIt);
						break;
					}

				EntityComponent.erase(e);
			}
		}
		Component* GetComponent(Entity e)
		{
			return (EntityComponent.find(e) != EntityComponent.end()) ? reinterpret_cast<Component*>(EntityComponent[e]) : nullptr;
		}

		std::vector<ComponentBlock<Component>*>* GetStorage()
		{
			return &Storage;
		}

		size_t GetCount() const
		{
			return EntityComponent.size();
		}

		std::unordered_map<Entity, uintptr_t>* GetEntityComponent()
		{
			return &EntityComponent;
		}

		std::type_index GetTypeIndex() const
		{
			return std::type_index(typeid(Component));
		}

	private:
		std::vector<std::unique_ptr<ComponentBlock<Component>>> Storage = {};
		std::unordered_map<Entity, uintptr_t> EntityComponent = {};
	};
}