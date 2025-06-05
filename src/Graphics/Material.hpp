#pragma once
#include "FileIO/Serializable.hpp"
#include <glm/glm.hpp>
namespace VulkanRenderer
{
	struct Texture;
	struct alignas(16) MaterialData
	{
		alignas(16) glm::vec3 mAmbient;
		alignas(16) glm::vec3 mDiffuse;
		alignas(16) glm::vec3 mSpecular;
		float mShininess;
		float mIor;
		int mIllum;

		//Idx of the textures to access inside the shader
		int32_t mAmbientTexIdx = -1;
		int32_t mDiffuseTexIdx = -1;
		int32_t mSpecularTexIdx = -1;
	};
	class Material
	{
	public:

		MaterialData mData;
		//data
		std::string mAmbientTexName;
		std::string mDiffuseTexName;
		std::string mSpecularTexName;
	};	
}