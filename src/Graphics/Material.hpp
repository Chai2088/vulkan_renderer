#pragma once
#include "FileIO/Serializable.hpp"
#include <glm/glm.hpp>
namespace VulkanRenderer
{
	struct Texture;
	struct alignas(16) MaterialData
	{
		glm::vec3 mAmbient;
		glm::vec3 mDiffuse;
		glm::vec3 mSpecular;
		float mShininess;
	};
	class Material : public ISerializable
	{
	public:
		void StreamRead(const nlohmann::json& j) override;
		void StreamWrite(nlohmann::json& j) const override;

		//data
		Texture* mDiffuseTexture;
		MaterialData mData;
	};	
}