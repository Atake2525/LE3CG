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

#pragma once

class DirectXBase;

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
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
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

private:
	DirectXBase* directxBase_ = nullptr;

	std::mt19937 randomEngine;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// DescriptorRange
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// RootParameter作成、PixelShaderのMatrixShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

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
	ModelData* modelData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexbufferView;



	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPilelineState = nullptr;

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

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

	struct ParticleGroup
	{
		std::string particleName;
		MaterialData materialData;
		std::list<Particle> particle;
		D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		uint32_t numInstance;
		ParticleForGPU* instancingData;
	};

	//MaterialData materialData;

	std::unordered_map<std::string, ParticleGroup> particleGroups;

};

