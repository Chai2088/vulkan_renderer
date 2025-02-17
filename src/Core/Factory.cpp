#include "Factory.hpp"
namespace VulkanRenderer
{
	void Factory::Shutdown()
	{
		for (auto& [idName, creator] : mCreators)
		{
			delete creator;
		}
	}
}
