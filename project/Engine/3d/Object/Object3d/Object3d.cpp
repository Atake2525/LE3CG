#define NOMINMAX
#include "Object3d.h"
#include "Object3dBase.h"
#include "DirectXBase.h"
#include "TextureManager.h"
#include "Light.h"
#include "kMath.h"
#include "WinApp.h"
#include "Model.h"
#include "ModelManager.h"
#include "CollisionManager.h"
#include "Camera.h"
#include <fstream>
#include <sstream>
#include <cassert>

#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

using namespace Microsoft::WRL;

void Object3d::Initialize() { 

	//// Resourceの作成
	CreateTransformationMatrixResrouce();
	//CreateLightResource();
	CreateCameraResource();

	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrix));
	// 書き込むためのアドレスを取得
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

	// 単位行列を書き込んでおく
	transformationMatrix->WVP = MakeIdentity4x4();
	transformationMatrix->World = MakeIdentity4x4();

	transform = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	first = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	aabb = {
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	SetAxisAngle({0.0f, 1.0f, 0.0f});

	SetQuaternionAngle(0.0f);

	cameraData->worldPosition = {1.0f, 1.0f, 1.0f};

	camera = Object3dBase::GetInstance()->GetDefaultCamera();


}

void Object3d::Update() {

	Matrix4x4 localMatrix = model_->GetModelData().rootNode.localMatrix;

	if (model_->IsAnimation())
	{
		animationTime += 1.0f / 60.0f; // 時刻を進める。1/60で固定してあるが、計測した時間を使って可変フレーム対応する方が望ましい
		animationTime = std::fmod(animationTime, animation.duration); // 最後まで行ったら最初からリピート再生。リピートしなくても別に良い
		NodeAnimation& rootNodeAnimation = animation.nodeAnimations[model_->GetModelData().rootNode.name]; // rootNodeのAnimationを取得
		Vector3 translate = CalculateValue(rootNodeAnimation.translate, animationTime);
		Quaternion rotate = CalculateValue(rootNodeAnimation.rotate, animationTime);
		Vector3 scale = CalculateValue(rootNodeAnimation.scale, animationTime);
		localMatrix = Multiply(localMatrix, MakeAffineMatrix(scale, rotate, translate));
		localMatrix = MakeAffineMatrix(scale, rotate, translate);
		if (animationTime >= 1)
		{
			Vector3 rot = { 0.0f, 0.0f, 0.0f };
		}
	}


	// 3DのTransform処理
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	worldMatrix = Multiply(worldMatrix, rotateQuaternionMatrix);

	if (isParent)
	{
		worldMatrix = Multiply(worldMatrix, parent);
	}

	Matrix4x4 worldViewProjectionMatrix;
	if (camera) {
		const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(Multiply(worldMatrix, localMatrix), viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = Multiply(worldMatrix, localMatrix);
	}
	
	transformationMatrix->WVP = worldViewProjectionMatrix;
	transformationMatrix->World = worldMatrix;

	Vector3 worldPos = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };

	aabb.min = first.min + worldPos;
	aabb.max = first.max + worldPos;

	ImGui::Begin("ObjectState");
	ImGui::SetWindowPos(ImVec2(0.0f, 18.0f));
	ImGui::SetWindowSize(ImVec2(300.0f, WinApp::kClientHeight - 18.0f));
	ImGui::DragFloat("AnimationTime", &animationTime);
	ImGui::End();

}

void Object3d::Draw() {
	
	if (model_) {
		model_->SetIA();
	}

	Object3dBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(4, Light::GetInstance()->GetDirectionalLightResource()->GetGPUVirtualAddress());

	Object3dBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(5, Light::GetInstance()->GetPointlLightResource()->GetGPUVirtualAddress());

	Object3dBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(6, Light::GetInstance()->GetSpotLightResource()->GetGPUVirtualAddress());

	// wvp用のCBufferの場所を設定
	Object3dBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	Object3dBase::GetInstance()->GetDxBase()->GetCommandList()->SetGraphicsRootConstantBufferView(3, cameraResource->GetGPUVirtualAddress());

	// 3Dモデルが割り当てられていれば描画する
	if (model_) {
		model_->Draw();
	}
}

void Object3d::CreateTransformationMatrixResrouce() { 
	transformationMatrixResource = Object3dBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(TransformationMatrix)); 
}

void Object3d::CreateCameraResource() { 
	cameraResource = Object3dBase::GetInstance()->GetDxBase()->CreateBufferResource(sizeof(CameraForGPU));
}

void Object3d::SetModel(const std::string& filePath) {
	// モデルを検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);
	CreateAABB();
	if (model_->IsAnimation())
	{
		animation = model_->GetAnimation();
	}
}

void Object3d::SetColor(const Vector4& color) { 
	model_->SetColor(color); 
}

const Vector4& Object3d::GetColor() const { 
	return model_->GetColor(); 
}

const bool Object3d::GetEnableLighting() const { 
	return model_->GetEnableLighting();
}

void Object3d::SetEnableLighting(const bool& enableLighting) { 
	model_->SetEnableLighting(enableLighting);
}

const Vector3 Object3d::GetRotateInDegree() const { 
	return SwapDegree(transform.rotate);
}

void Object3d::SetRotateInDegree(const Vector3& rotate) { 
	transform.rotate = SwapRadian(rotate);
}

void Object3d::SetTransform(const Vector3& translate, const Vector3& scale, const Vector3& rotate) {
	transform.translate = translate;
	transform.scale = scale;
	transform.rotate = rotate;
}

const float& Object3d::GetShininess() const { 
	return model_->GetShininess();
}

void Object3d::SetShininess(const float& shininess) { 
	model_->SetShininess(shininess);
}

void Object3d::CreateAABB() {
	const std::vector<VertexData> vData = model_->GetVertices();
	
	for (VertexData vertices : vData)
	{
		first.min.x = std::min(first.min.x, vertices.position.x);
		first.min.y = std::min(first.min.y, vertices.position.y);
		first.min.z = std::min(first.min.z, vertices.position.z);

		first.max.x = std::max(first.max.x, vertices.position.x);
		first.max.y = std::max(first.max.y, vertices.position.y);
		first.max.z = std::max(first.max.z, vertices.position.z);
	}
}

const bool& Object3d::CheckCollision(Object3d* object) const {
	return CollisionAABB(aabb, object->GetAABB());
}