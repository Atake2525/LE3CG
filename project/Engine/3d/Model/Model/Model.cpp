#include "Model.h"
#include "ModelBase.h"
#include "DirectXBase.h"
#include "kMath.h"
#include "TextureManager.h"
#include "Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Logger;


void Model::Initialize(std::string directoryPath, std::string filename, const bool enableLighting, const bool isAnimation) {
	// モデル読み込み
	modelData = LoadModelFile(directoryPath, filename);

	if (isAnimation)
	{
		this->isAnimation = true;
		animation = LoadAnimationFile(directoryPath, filename);
	}

	// Resourceの作成
	CreateVertexResource();
	CreateMaterialResouce();

	// BufferResourceの作成
	CreateVertexBufferView();

	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点データをリソースにコピー
	//  書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// データを書き込む
	// 今回は赤を書き込んでみる
	materialData->color = {1.0f, 1.0f, 1.0f, 1.0f};

	materialData->uvTransform = MakeIdentity4x4();

	materialData->enableLighting = enableLighting;
	materialData->shininess = 70.0f;
	materialData->specularColor = {1.0f, 1.0f, 1.0f};

	// テクスチャ読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	// 読み込んだテクスチャの番号尾を取得
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);
}

void Model::SetIA() {
	// ModelTerrain
	ModelBase::GetInstance()->GetDxBase()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
}

void Model::Draw() {

	// wvp用のCBufferの場所を設定
	ModelBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	ModelBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureIndex));

	ModelBase::GetInstance()->GetDxBase()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	// 1, 中で必要となる変数の宣言
	MaterialData materialData; // 構築するMaterialData
	std::string line;          // ファイルから読んだ１行を格納するもの
	// 2, ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());                             // とりあえず開けなかったら止める
	// 3, 実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
		else
		{
			// map_Kdが存在しなかったらwhite1x1をテクスチャとして使用する
			materialData.textureFilePath = "Resources/Debug/white1x1.png";
		}
	}
	// 4, MaterialDataを返す
	return materialData;
}

// マルチスレッド化予定
ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
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

Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
	Animation animation; // 保管するアニメーション
	Assimp::Importer importer;
	// ファイルを格納
	std::string filePath = directoryPath + "/" + filename;
	// シーンを読む
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	// アニメーションが無い
	if (scene->mNumAnimations == 0) {
		Log("this Scene have not animation");
		assert(0);
	}
	aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションだけ採用。複数対応するに越したことはない
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

	/// NodeAnimationを解析する
	// assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報をとってくる
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex)
	{
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimation[nodeAnimationAssimp->mNodeName.C_Str()];
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyFrameVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.translate.KeyFrames.push_back(keyframe);
		}
		// RotateはmNumRotationKeys/mRotationKeys, ScaleはmNumScalingKeys/mScaliongKeysで取得できるので同様に行う
		// RotateはQuaternionで、右手->左手に変換するために、yとzを反転させる必要がある。Scaleはそのままで良い。
		// keyframe.value = {rotate.x, -rotate.y, -rotate.z, rotate.w};
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
		{
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyFrameQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
			nodeAnimation.rotate.KeyFrames.push_back(keyframe);
		}
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
		{
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyFrameVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.translate.KeyFrames.push_back(keyframe);
		}
		animation.nodeAnimation[nodeAnimationAssimp->mNodeName.C_Str()] = nodeAnimation;
		animation.nodeAnimationName = nodeAnimationAssimp->mNodeName.C_Str();
	}
	// 解析したアニメーションを返す
	return animation;
}


void Model::CreateVertexResource() {
	// 頂点リソースの作成
	vertexResource = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
}

void Model::CreateVertexBufferView() {
	// 頂点バッファビューを作成する
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点サイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);                                 // １頂点あたりのサイズ
}

void Model::CreateMaterialResouce() { 
	materialResource = ModelBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(Material)); 
}