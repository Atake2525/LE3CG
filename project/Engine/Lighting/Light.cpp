#include "Light.h"
#include "kMath.h"
#include "DirectXBase.h"

Light* Light::instance = nullptr;

Light* Light::GetInstance() {
	if (instance == nullptr) {
		instance = new Light;
	}
	return instance;
}

void Light::Finalize() {
	delete instance;
	instance = nullptr;
}

void Light::Initialize(DirectXBase* directxBase) {

	directxBase_ = directxBase;

	// ライト関係の初期化
	directionalLightResource = directxBase_->CreateBufferResource(sizeof(DirectionalLight));

	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 0.1f;
	directionalLightData->specularColor = { 0.0f, 0.0f, 0.0f };

	pointLightResource = directxBase_->CreateBufferResource(sizeof(PLight));

	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));


	pointLightData->light[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->light[0].position = { 0.0f, 2.0f, 0.0f };
	pointLightData->light[0].intensity = 1.0f;
	pointLightData->light[0].radius = 10.0f;
	pointLightData->light[0].dacay = 5.0f;
	pointLightData->light[0].specularColor = { 1.0f, 1.0f, 1.0f };

	pointLightData->light[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	pointLightData->light[1].position = { 0.0f, 1.0f, 0.0f };
	pointLightData->light[1].intensity = 1.0f;
	pointLightData->light[1].radius = 10.0f;
	pointLightData->light[1].dacay = 5.0f;
	pointLightData->light[1].specularColor = { 1.0f, 1.0f, 1.0f };

	pointLightData->lightCount = 2;

	spotLightResource = directxBase_->CreateBufferResource(sizeof(SpotLight));

	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));

	spotLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	spotLightData->position = { 2.0f, 1.25f, 0.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = Normalize({ -1.0f, -1.0f, 0.0f });
	spotLightData->intensity = 0.0f;
	spotLightData->dacay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 2.6f);
	spotLightData->specularColor = { 1.0f, 1.0f, 1.0f };
}