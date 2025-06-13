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
		alignas(16) glm::vec3	mPosition;
		alignas(16) glm::vec3	mColor;
		alignas(16) glm::vec3	mDirection;
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
		void Edit() override;

		//Serialization
		void StreamRead(const nlohmann::json& j) override;
		void StreamWrite(nlohmann::json& j) const override;
	
		LightData mData;
	};
}