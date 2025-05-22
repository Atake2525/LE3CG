#include <random>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <list>
#include "Transform.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Model.h"
#include <dxcapi.h>
#include <unordered_map>
#include "AABB.h"

#pragma once

class DirectXBase;
class Camera;

struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct AccelerationField {
	Vector3 acceleration;
	AABB area;
};

struct ParticleFlag
{
	bool isAccelerationField;
	bool start;
};

struct ParticleGroup
{
	std::string particleName;
	AccelerationField accelerationField;
	ParticleFlag particleFlag;
	MaterialData materialData;
	std::list<Particle> particles;
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
	uint32_t numInstance;
	ParticleForGPU* instancingData;
};

struct Emitter {
	Transform transform; //!< エミッタのTransform
	uint32_t count; //!< 発生数
	float frequency; //!< 発生頻度
	float frequencyTime; //!< 頻度用時刻
};

class ParticleManager {
private:
	// シングルトンパターンを適用
	static ParticleManager* instance;

	// コンストラクタ、デストラクタの隠蔽
	ParticleManager() = default;
	~ParticleManager() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;

public:
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	/// <returns></returns>
	static ParticleManager* GetInstance();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXBase* directxBase);

	ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="name">名前</param>
	/// <param name="textureFilePath">テクスチャ名</param>
	void CreateParticleGroup(const std::string& name, const std::string& textureFilePath);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	void Emit(const std::string name, const Vector3& position, uint32_t count);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// Getter(Camera)
	Camera* GetCamera() const { return camera; }

	// Setter(Camera)
	void SetCamera(Camera* camera) { this->camera = camera; }

private:

	Camera* camera = nullptr;
	/// <summary>
	/// ランダムエンジンの初期化
	/// </summary>
	void InitializeRandomEngine();
	/// <summary>
	/// ルートシグネチャ作成
	/// </summary>
	void CreateRootSignature();
	/// <summary>
	/// パイプラインの生成
	/// </summary>
	void CreateGraphicsPipeLineState();
	/// <summary>
	/// 頂点データの初期化
	/// </summary>
	void InitializeVetexData();
	/// <summary>
	/// 頂点リソースの生成
	/// </summary>
	void CreateVertexResource();
	/// <summary>
	/// 頂点バッファビューの生成
	/// </summary>
	void CreateVertexxBufferView();
	/// <summary>
	/// 頂点リソースに頂点データを書き込む
	/// </summary>
	void MappingVertexData();

	/// <summary>
	///  マテリアル利ソールの作成
	/// </summary>
	void CreateMaterialResource();


private:
	DirectXBase* directxBase_ = nullptr;

	std::mt19937 randomEngine;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// バイナリをもとに作成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;


	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	/// GraphicsPipeLineState
	// PSOを作成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	VertexData* vertexData = nullptr;

	ModelData modelData;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexbufferView;



	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	// △tを定義。とりあえず60fps固定してあるが、実時間を計算して可変fpsで動かせるようにしておくとなお良い
	const float deltaTime = 1.0f / 60.0f;

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};


	const uint32_t maxNumInstance = 100;


	bool IsCollision(const AABB& aabb, const Vector3& point);
	//MaterialData materialData;
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	//MaterialData materialData;
	Particle MakeNewParticle_HitEffect(std::mt19937& randomEngine, const Vector3& translate);

	std::unordered_map<std::string, ParticleGroup> particleGroups;

};

