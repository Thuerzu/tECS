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

namespace tECS {
	struct IComponentStorageBase {
	public:
		virtual Entity get_entity_of_component(uintptr_t ptr) const = 0;
		virtual void remove_entity(Entity e) = 0;
		virtual size_t get_count() const = 0;
		virtual bool has_entity(Entity e) const = 0;
		virtual std::unordered_map<Entity, uintptr_t>* get_entity_component() = 0;
		virtual std::type_index get_type_index() const = 0;
	};

	template <typename Component>
	struct ComponentBlock {
	public:
		ComponentBlock() = default;
		
		uintptr_t add(Entity e, const Component& comp) {
			if (full())
				return 0;
			size_t index = tombstones.CountrOne();
			storage[index] = comp;
			tombstones.Set(index);
			entities[index] = e;
			return reinterpret_cast<uintptr_t>(&storage[index]);
		}

		template <typename... Args>
		uintptr_t emplace(Entity e, Args&&... args) {
			if (full())
				return 0;
			size_t index = tombstones.CountrOne();
			storage[index] = Component{ std::forward<Args>(args)... };
			tombstones.Set(index);
			entities[index] = e;
			return reinterpret_cast<uintptr_t>(&storage[index]);
		}

		Component* get(Entity e) {
			for (size_t i = 0; i < storage.size(); i++)
				if (tombstones.Test(i) && entities[i] == e)
					return &storage[i];
			return nullptr;
		}

		bool remove(Entity e) {
			for (size_t i = 0; i < storage.size(); i++)
				if (tombstones.Test(i) && entities[i] == e) {
					tombstones.Reset(i);
					return true;
				}
			return false;
		}

		bool remove_at(uintptr_t ptr) {
			uintptr_t base = reinterpret_cast<uintptr_t>(&storage[0]);

			if (ptr < base || ptr >= base + sizeof(Component) * storage.size())
				return false;

			size_t index = (ptr - base) / sizeof(Component);
			tombstones.Reset(index);
			return true;
		}

		Entity get_entity_of_component(uintptr_t ptr) {
			uintptr_t base = reinterpret_cast<uintptr_t>(&storage[0]);
			if (ptr < base || ptr >= base + sizeof(Component) * storage.size())
				return 0;

			size_t index = (ptr - base) / sizeof(Component);
			if (tombstones.Test(index))
				return entities[index];
			return 0;
		}

		bool full() const {
			return tombstones.All();
		}

		bool empty() const {
			return tombstones.None();
		}

	public:
		std::array<Component, 1024> storage;
		std::array<Entity, 1024> entities;
		Bitset<1024> tombstones;
	};

	template <typename Component>
	struct ComponentStorage : public IComponentStorageBase {
	public:

		ComponentStorage() = default;
		bool has_entity(Entity e) const {
			return (entity_component.find(e) != entity_component.end());
		}
		Entity get_entity_of_component(uintptr_t comp) const {
			for (const auto& pair : entity_component)
				if (reinterpret_cast<uintptr_t>(pair.second) == comp)
					return pair.first;
			return 0;
		}
		void add_entity(Entity e, const Component& comp) {
			for (auto& block : storage) {
				if (auto compPtr = block->Add(e, comp)) {
					entity_component[e] = compPtr;
					return;
				}
			}
			//No space found, create new block
			auto new_block = std::make_unique<ComponentBlock<Component>>();
			auto comp_ptr = new_block->Add(e, comp);
			storage.push_back(std::move(new_block));
			entity_component[e] = comp_ptr;
		}
		void add_or_modify_entity(Entity e, const Component& comp)
		{
			if (has_entity(e)) {
				*reinterpret_cast<Component*>(entity_component[e]) = comp;
				return;
			}
			add_entity(e, comp);
		}

		template <typename... Args>
		void emplace_entity(Entity e, Args&&... args) {
			for (auto& block : storage) {
				if (auto compPtr = block->emplace(e, std::forward<Args>(args)...)){
					entity_component[e] = compPtr;
					return;
				}
			}
			//No space found, create new block
			THLIB_SET_MARKER("NEW BLOCK");
			auto newBlock = std::make_unique<ComponentBlock<Component>>();
			auto comp = newBlock->emplace(e, std::forward<Args>(args)...);
			storage.push_back(std::move(newBlock));
			entity_component[e] = comp;
		}

		template<typename... Args>
		void emplace_or_modify_entity(Entity e, Args&&... args) {
			if (has_entity(e)) {
				*reinterpret_cast<Component*>(entity_component[e]) = Component{ std::forward<Args>(args)... };
				return;
			}
			emplace_entity(e, std::forward<Args>(args)...);
		}

		void remove_entity(Entity e) {
			if (storage.empty())
				return;

			auto i = entity_component.find(e);
			if (i == entity_component.end())
				return;

			for (auto blockIt = storage.begin(); blockIt < storage.end(); blockIt++) {
				if ((*blockIt)->remove_at(i->second)) {
					if ((*blockIt)->empty())
						blockIt = storage.erase(blockIt);
					break;
				}
			}
			entity_component.erase(e);
		}
		Component* get_component(Entity e) {
			return (entity_component.find(e) != entity_component.end()) ? reinterpret_cast<Component*>(entity_component[e]) : nullptr;
		}

		std::vector<ComponentBlock<Component>*>* get_storage() {
			return &storage;
		}

		size_t get_count() const {
			return entity_component.size();
		}

		std::unordered_map<Entity, uintptr_t>* get_entity_component() {
			return &entity_component;
		}

		std::type_index get_type_index() const {
			return std::type_index(typeid(Component));
		}

	private:
		std::vector<std::unique_ptr<ComponentBlock<Component>>> storage = {};
		std::unordered_map<Entity, uintptr_t> entity_component = {};
	};
}