#include "Utils.hpp"
namespace VulkanRenderer
{
	std::string ParseIDName(const char* idName)
	{
		std::string typeName = idName;

		size_t first = typeName.find_last_of("::");
		if (first == std::string::npos)
		{
			first = typeName.find_last_of(" ");
		}
		typeName = typeName.substr(first + 1, typeName.size() - first);

		return typeName;
	}
}