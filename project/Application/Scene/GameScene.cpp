#include "GameScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

void GameScene::Initialize() {

	ModelManager::GetInstance()->LoadModel("Resources/Model/obj", "terrain.obj");

	camera = new Camera();
	camera->SetRotate(Vector3(0.36f, 0.0f, 0.0f));

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera);

	object3d = new Object3d();
	object3d->Initialize();
	object3d->SetModel("terrain.obj");

	sprite = new Sprite();
	sprite->Initialize("Resources/Debug/white1x1.png");
	leftTop = { 0.0f, 0.0f };
	transformSprite = {
		{200.0f, 200.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{640.0f, 360.0f, 1.0f}
	};
	sprite->SetTransform(transformSprite);
	sprite->SetTextureLeftTop(leftTop);

	cameraTransform.scale = { 1.0f, 1.0f, 1.0f };

	cameraTransform.rotate.x = 0.39f;
	cameraTransform.translate = { -0.34f, 10.0f, -27.0f };

	modelTransform = object3d->GetTransform();
}

void GameScene::Update() {

	ImGui::Begin("State");
	if (ImGui::TreeNode("Camera")) {
		ImGui::DragFloat3("Tranlate", &cameraTransform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &cameraTransform.scale.x, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Sprite")) {
		ImGui::DragFloat3("Tranlate", &transformSprite.translate.x, 1.0f);
		ImGui::DragFloat("Rotate", &transformSprite.rotate.z, 0.01f);
		ImGui::DragFloat3("Scale", &transformSprite.scale.x, 1.0f);
		ImGui::DragFloat2("Min", &leftTop.x, 0.1f);
		ImGui::TreePop();
	}
	ImGui::End();

	if (input->TriggerKey(DIK_ESCAPE))
	{
		finished = true;
	}
	const float speed = 0.7f;
	Vector3 velocity(0.0f, 0.0f, speed);
	velocity = TransformNormal(velocity, camera->GetWorldMatrix());
	if (input->PushKey(DIK_W)) {
		cameraTransform.translate += velocity;
	}
	if (input->PushKey(DIK_S)) {
		cameraTransform.translate -= velocity;
	}
	velocity = { speed, 0.0f, 0.0f };
	velocity = TransformNormal(velocity, camera->GetWorldMatrix());
	if (input->PushKey(DIK_A)) {
		cameraTransform.translate -= velocity;
	}
	if (input->PushKey(DIK_D)) {
		cameraTransform.translate += velocity;
	}
	if (input->PushKey(DIK_SPACE)) {
		cameraTransform.translate.y += 1.0f;
	}
	if (input->PushKey(DIK_LSHIFT)) {
		cameraTransform.translate.y -= 1.0f;
	}
	if (input->PushKey(DIK_LEFT)) {
		cameraTransform.rotate.y -= 0.03f;
	}
	if (input->PushKey(DIK_RIGHT)) {
		cameraTransform.rotate.y += 0.03f;
	}
	if (input->PushKey(DIK_UP)) {
		cameraTransform.rotate.x -= 0.03f;
	}
	if (input->PushKey(DIK_DOWN)) {
		cameraTransform.rotate.x += 0.03f;
	}
	if (input->PushKey(DIK_Q)) {
		cameraTransform.rotate.z -= 0.01f;
	}
	if (input->PushKey(DIK_E)) {
		cameraTransform.rotate.z += 0.01f;
	}


	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
	camera->Update();
	object3d->SetTransform(modelTransform);
	object3d->SetEnableLighting(enableLighting);
	object3d->Update();
	aabb = object3d->GetAABB();
	sprite->SetTransform(transformSprite);
	sprite->SetTextureLeftTop(leftTop);
	sprite->TriangleUpdate();
	
	input->Update();

}

void GameScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();

	//sprite->Draw();

	Object3dBase::GetInstance()->ShaderDraw();

	object3d->Draw();

}

void GameScene::Finalize() {

	delete camera;

	delete object3d;

	delete sprite;

}