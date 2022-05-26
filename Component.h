#pragma once

namespace tECS
{
	template <typename T>
	class Component
	{
	public:
		using DataType = T;
		int ID;
	};
}
