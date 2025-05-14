#include "ParticleManager.h"
#include "DirectXBase.h"
#include <cassert>
#include "Logger.h"
#include "TextureManager.h"
#include "kMath.h"
#include "Camera.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


using namespace Logger;

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr)
	{
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ParticleManager::Initialize(DirectXBase* directxBase) {
	directxBase_ = directxBase;
	InitializeRandomEngine();
	CreateGraphicsPipeLineState();
	CreateVertexResource();
	CreateVertexxBufferView();
	MappingVertexData();
	InitializeVetexData();
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& textureFilePath) {
	auto it = particleGroups.find(name);
	if (it != particleGroups.end())
	{
		// 読み込み済なら早期return
		return;
	}

	// パーティクルの作成
	ParticleGroup group;
	group.numInstance = 0;
	group.particleName = name;
	group.materialData.textureFilePath = textureFilePath;
	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	group.materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	group.instancingResource = directxBase_->CreateBufferResource(sizeof(ParticleForGPU) * group.numInstance);
	group.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&group.instancingData));
	for (uint32_t index = 0; index < group.numInstance; index++)
	{
		group.instancingData[index].WVP = MakeIdentity4x4();
		group.instancingData[index].World = MakeIdentity4x4();
	}
	group.particleFlag.isAccelerationField = false;
	group.particleFlag.start = false;

	group.accelerationField.acceleration = { 0.0f, 0.0f, 0.0f };
	group.accelerationField.area = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = maxNumInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = directxBase_->GetSRVCPUDescriptorHandle(3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = directxBase_->GetSRVGPUDescriptorHandle(3);
	directxBase_->GetDevice()->CreateShaderResourceView(group.instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	group.instancingSrvDesc = instancingSrvDesc;

	particleGroups.at(name) = group;

}

void ParticleManager::Update() {
	
	// CG3_01_02
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	// 一旦常にBillboardするようにしておく
	//if (!useBillboard) {
	billboardMatrix = MakeIdentity4x4();
	//}

	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroup = particleGroups.begin(); particleGroup != particleGroups.end(); ++particleGroup)
	{
		for (std::list<Particle>::iterator particleIterator = particleGroup->second.particle.begin(); particleIterator != particleGroup->second.particle.end();) {
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup->second.particle.erase(particleIterator); // 生存時間が過ぎたParticleはlistから消す。戻り値が次のイテレータとなる
				continue;
			}
			// Fieldの範囲内のParticleには加速度を適用する
			if (particleGroup->second.particleFlag.isAccelerationField) {
				if (IsCollision(particleGroup->second.accelerationField.area, (*particleIterator).transform.translate)) {
					(*particleIterator).velocity += particleGroup->second.accelerationField.acceleration * deltaTime;
				}
			}
			//(*particleIterator).currentTime = (*particleIterator).currentTime;
			(*particleIterator).currentTime += deltaTime;
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			(*particleIterator).transform.translate += (*particleIterator).velocity * deltaTime;
			if (particleGroup->second.particleFlag.start) {
				// ...WorldMatrixを求めたり
				float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
				(*particleIterator).transform.translate += (*particleIterator).velocity * deltaTime;
				(*particleIterator).currentTime += deltaTime;
			}

			Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);
			Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));
			//Matrix4x4 worldMatrix = MakeAffineMatrix(particles[index].transform.scale, particles[index].transform.rotate, particles[index].transform.translate);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, camera->GetViewProjectionMatrix());
			// インスタンスが最大数を超えないようにする
			if (particleGroup->second.numInstance < maxNumInstance) {
				particleGroup->second.instancingData[particleGroup->second.numInstance].WVP = worldViewProjectionMatrix;
				particleGroup->second.instancingData[particleGroup->second.numInstance].World = worldMatrix;
				particleGroup->second.instancingData[particleGroup->second.numInstance].color = (*particleIterator).color;
				particleGroup->second.instancingData[particleGroup->second.numInstance].color.w = alpha;
				++particleGroup->second.numInstance;
			}
			++particleIterator;
		}
	}
}

void ParticleManager::Draw() {
	// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directxBase_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// PSOを設定
	directxBase_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get()); 
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	directxBase_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// VBVを設定
	directxBase_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);

	// 残りのパーティクルのDraw処理
}

void ParticleManager::InitializeRandomEngine() {
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
}

void ParticleManager::CreateRootSignature() {
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	descriptorRangeForInstancing[0].BaseShaderRegister = 0; // 0から始める
	descriptorRangeForInstancing[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Samplerの設定
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;   // バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;     // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                       // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;                               // レジスタ番号0
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;              // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                              // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescroptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;          // VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数

	// シリアライズしてバイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリをもとに作成
	hr = directxBase_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	// InputLayout
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// NomalBlendを行うための設定
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	// RasiterzerStateの設定
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	vertexShaderBlob = directxBase_->CompileShader(L"Resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	pixelShaderBlob = directxBase_->CompileShader(L"Resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void ParticleManager::CreateGraphicsPipeLineState() {
	CreateRootSignature();
	// PSOを作成する
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();                                           // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                  // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };   // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                         // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                               // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ、三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくて良い)
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	HRESULT hr = directxBase_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

}

void ParticleManager::InitializeVetexData() {
	vertexData[0].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexData[0].texcoord = { 0.0f, 0.0f };
	vertexData[1].position = { 1.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexData[1].texcoord = { 1.0f, 0.0f };
	vertexData[2].position = { 1.0f, 1.0f, 0.0f, 1.0f }; // 右下
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[3].position = { 0.0f, 1.0f, 0.0f, 1.0f }; // 左下
	vertexData[3].texcoord = { 0.0f, 1.0f };
}

// マルチスレッド化予定
ModelData ParticleManager::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate);
	assert(scene->HasMeshes()); // メッシュが無いのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線が無いMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordsがないMeshは今回は非対応
		// ここからMeshの中身(Face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
		{
			aiFace& face = mesh->mFaces[faceIndex];

			assert(face.mNumIndices == 3); // 3角形のみサポート
			// ここからFaceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element)
			{
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };
				// aiProcess_MakeLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}

		}
	}
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
		else
		{
			modelData.material.textureFilePath = "Resources/Debug/white1x1.png";
		}
	}
	return modelData;
}

void ParticleManager::CreateVertexResource() {
	// 頂点リソースの作成
	vertexResource = directxBase_->CreateBufferResource(sizeof(VertexData) * modelData->vertices.size());
}

void ParticleManager::CreateVertexxBufferView() {
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * modelData->vertices.size();
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

void ParticleManager::MappingVertexData() {
	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
}

bool ParticleManager::IsCollision(const AABB& aabb, const Vector3& point) {
	if ((aabb.min.x <= point.x && aabb.max.x >= point.x) &&
		(aabb.min.y <= point.y && aabb.max.y >= point.y) &&
		(aabb.min.z <= point.z && aabb.max.z >= point.z)) {
		return true;
	}
	return false;
}