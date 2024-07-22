#include <inttypes.h>
#include <unordered_map>
#include "Core.h"
namespace tECS
{
	using Entity = uint64_t;			//This is used as an ID; 0 is empty, starts at 1

	struct TECS_API IComponent
	{
		virtual ~IComponent() = default;
	};

	struct TECS_API ISystem
	{
		virtual void Init() = 0;
		virtual void Update() = 0;
		virtual ~ISystem() = default;
	};

	struct TECS_API IBaseComponentStorage
	{
	public:
		virtual size_t GetIndexOfEntity(Entity e) = 0;
		virtual Entity GetEntityAtIndex(size_t i) = 0;
		virtual void RemoveEntity(Entity e) = 0;
	};

	template <typename Component>
	struct ComponentStorage : public IBaseComponentStorage
	{
	public:
		size_t GetIndexOfEntity(Entity e)
		{ 
			if (EntityToIndex.find(e) != EntityToIndex.end())
				return EntityToIndex.at(e);
			return 0;
		}
		Entity GetEntityAtIndex(size_t i) 
		{
			return IndexToEntity[i];
		}
		void AddEntity(Entity e, const Component& comp)
		{
			if (DeletedComponents.size() > 0)
			{
				IndexToEntity[DeletedComponents.back()] = e;
				EntityToIndex[e] = DeletedComponents.back();
				Storage[DeletedComponents.back()] = comp;
				DeletedComponents.erase(DeletedComponents.end());
			}
			else
			{
				Storage.push_back(comp);
				IndexToEntity.push_back(e);
				EntityToIndex[e] = Storage.size() - 1;
			}
		}
		void RemoveEntity(Entity e)
		{
			if (EntityToIndex.find(e) != EntityToIndex.end())
			{
				DeletedComponents.push_back(EntityToIndex.at(e));
				Storage.erase(Storage.begin() + DeletedComponents.back());
				IndexToEntity[DeletedComponents.back()] = 0;
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
	private:
		ComponentStorage() 
		{
			Storage = std::vector<Component>();
			IndexToEntity = std::vector<Entity>();
			DeletedComponents = std::vector<size_t>();
			EntityToIndex = std::unordered_map<Entity, size_t>();
		};
	public:
		static ComponentStorage<Component>& Get()
		{
			return s_Instance;
		}

	private:
		std::vector<Component> Storage;
		std::unordered_map<Entity, size_t> EntityToIndex;
		std::vector<Entity> IndexToEntity;
		std::vector<size_t> DeletedComponents;
	private:
		static ComponentStorage<Component> s_Instance;
	};

	template <typename Component>
	ComponentStorage<Component> ComponentStorage<Component>::s_Instance = ComponentStorage<Component>();
}