#include <wrl.h>
#include <d3d12.h>
#include "Vector3.h"
#include "Vector4.h"

#pragma once

struct DirectionalLight {
	Vector4 color;     //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity;   //!< 輝度
	Vector3 specularColor;
	float padding[2];
};

struct PointLight {
	Vector4 color;    //!< ライトの色
	Vector3 position; //!< ライトの位置
	float intensity;  //!< 輝度
	float radius;     //!< ライトの届く最大距離
	float dacay;      //!< 減衰率
	Vector3 specularColor;
	float padding[2];
};

struct SpotLight {
	Vector4 color;         //!< ライトの色
	Vector3 position;      //!< ライトの位置
	float intensity;       //!< 輝度
	Vector3 direction;     //!< ライトの向き
	float distance;        //!< ライトの届く最大距離
	float dacay;           //!< 減衰率
	float cosAngle;        //!< スポットライトの余弦
	float cosFalloffStart; // falloffが開始される角度
	Vector3 specularColor;
	float padding[2];
};

class DirectXBase;

class Light {
	// シングルトンパターンを適用
	static Light* instance;

	// コンストラクタ、デストラクタの隠蔽
	Light() = default;
	~Light() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	Light(Light&) = delete;
	Light& operator=(Light&) = delete;
public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>Light* instance</returns>
	static Light* GetInstance();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	void Initialize(DirectXBase* directxBase);

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetDirectionalLightResource() const { return directionalLightResource; }

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetPointlLightResource() const { return pointLightResource; }

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetSpotLightResource() const { return spotLightResource; }

private:
	// ライトリソース宣言
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;

	DirectionalLight* directionalLightData = nullptr;

	PointLight* pointLightData = nullptr;

	SpotLight* spotLightData = nullptr;

	DirectXBase* directxBase_ = nullptr;
};

