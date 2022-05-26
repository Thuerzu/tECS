#pragma once
namespace tECS
{
	class Entity
	{
	public:
		template<typename ComponentType>
		bool HasComponent();

	public:
		int ID;
	};
}
