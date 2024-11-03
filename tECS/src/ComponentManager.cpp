#include "ComponentManager.h"

namespace tECS
{
	Entity ECS::CreateEntity()
	{
		if (DeletedEntities.empty()) return ++Next;
		Entity temp = DeletedEntities.back();
		DeletedEntities.pop_back();
		return temp;
	}

	void ECS::DeleteEntity(Entity& e)
	{
		for (auto compStorage : ComponentsData)
			compStorage.second->RemoveEntity(e);
		DeletedEntities.push_back(e);
	}

	void ECS::RegisterSystem(ISystem* sys)
	{
		Systems.push_back(sys);
		sys->Init();
	}

	inline double ECS::GetDeltaTime() { return DeltaTime; }

	void ECS::UpdateSystems(double deltaTime)
	{
		DeltaTime = deltaTime;
		for (auto sys : Systems)
		{
			sys->Update();
		}
	}
}