#include "Model.h"
#include "ModelBase.h"
#include "DirectXBase.h"
#include "kMath.h"
#include "TextureManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Model::Initialize(std::string directoryPath, std::string filename, bool enableLighting) {
	// モデル読み込み
	modelData = LoadModelFile(directoryPath, filename);

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
	//// 1. 中で必要となる変数の宣言
	//std::vector<Vector4> positions; // 位置
	//std::vector<Vector3> normals;   // 法線
	//std::vector<Vector2> texcoords; // テクスチャ座標
	//std::string line;               // ファイルから読んだ1行を格納するもの

	//// 2. ファイルを開く
	//std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	//assert(file.is_open());                             // とりあえず開けなかったら止める
	//// 3. 実際にファイルを読み、ModelDataを構築していく
	//while (std::getline(file, line)) {
	//	std::string identifier;
	//	std::istringstream s(line);
	//	s >> identifier; // 先頭の識別子を読む

	//	// identifierに応じた処理
	//	if (identifier == "v") {
	//		Vector4 position;
	//		s >> position.x >> position.y >> position.z;
	//		position.w = 1.0f;
	//		positions.push_back(position);
	//	} else if (identifier == "vt") {
	//		Vector2 texcoord;
	//		s >> texcoord.x >> texcoord.y;
	//		texcoords.push_back(texcoord);
	//	} else if (identifier == "vn") {
	//		Vector3 normal;
	//		s >> normal.x >> normal.y >> normal.z;
	//		normals.push_back(normal);
	//	} else if (identifier == "f") {
	//		VertexData triangle[3];

	//		// 面は三角形限定。その他は未対応
	//		for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
	//			std::string vertexDefinition;
	//			s >> vertexDefinition;
	//			// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
	//			std::istringstream v(vertexDefinition);
	//			uint32_t elementIndices[3];
	//			for (int32_t element = 0; element < 3; ++element) {
	//				std::string index;
	//				std::getline(v, index, '/'); // /区切りでインデックスを読んでいく
	//				elementIndices[element] = std::stoi(index);
	//			}
	//			// 要素へのIndexから、実際の要素を値を取得して、頂点を構築する
	//			Vector4 position = positions[elementIndices[0] - 1];
	//			Vector2 texcoord = texcoords[elementIndices[1] - 1];
	//			Vector3 normal = normals[elementIndices[2] - 1];
	//			// VertexData vertex = { position, texcoord, normal };
	//			// modelData.vertices.push_back(vertex);
	//			position.x *= -1.0f;
	//			//position.y *= -1.0f;
	//			normal.x *= -1.0f;
	//			texcoord.y = 1.0f - texcoord.y;

	//			triangle[faceVertex] = {position, texcoord, normal};
	//		}
	//		// 頂点を逆順で登録することで、周り順を逆にする
	//		modelData.vertices.push_back(triangle[2]);
	//		modelData.vertices.push_back(triangle[1]);
	//		modelData.vertices.push_back(triangle[0]);
	//	} else if (identifier == "mtllib") {
	//		// materialTemplateLibraryファイルの名前を取得する
	//		std::string materialFilename;
	//		s >> materialFilename;
	//		// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
	//		modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
	//	}
	//}
	//// 4. ModelDataを返す
	//return modelData;
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