#pragma once
#include "Scene/Component.hpp"
#include "FileIO/Serializable.hpp"
#include <glm/glm.hpp>
namespace VulkanRenderer
{
	enum eLightType {ePOINT, eDIRECTIONAL};
	struct alignas(16) LightData
	{
		LightData();
		glm::vec3	mPosition;
		glm::vec3	mDirection;
		glm::vec3	mColor;
		int			mType;
		float		mIntensity;
		float		mRadius;
	};
	
	class Light : public Component, public ISerializable
	{
	public:
		Light();
		void OnCreate() override;
		void Initialize() override;
		void Shutdown() override;

		//Serialization
		void StreamRead(const nlohmann::json& j) override;
		void StreamWrite(nlohmann::json& j) const override;
	
		LightData mData;
	};
}