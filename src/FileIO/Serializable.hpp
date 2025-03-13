#pragma once
#include <nlohmann/json.hpp>
namespace VulkanRenderer
{
	//Interface class to serialize components and resources
	class ISerializable
	{
	public:
		virtual void StreamRead(const nlohmann::json& j) = 0;
		virtual void StreamWrite(nlohmann::json& j) const = 0;
	};
}