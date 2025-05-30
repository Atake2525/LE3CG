#include "GameScene.h"
#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

void GameScene::Initialize() {

	ModelManager::GetInstance()->LoadModel("Resources/Model/obj", "terrain.obj");

	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");

	ParticleManager::GetInstance()->CreateParticleGroup("hit", "Resources/Model/obj/Circle2.png", ParticleType::Ring);
	ParticleManager::GetInstance()->CreateParticleGroup("slash", "Resources/Model/obj/Circle2.png", ParticleType::Ring);

	camera = new Camera();
	camera->SetRotate(Vector3(0.36f, 0.0f, 0.0f));
	camera->SetTranslate({ 0.0f, 5.0f, -10.0f });

	ParticleManager::GetInstance()->SetCamera(camera);

	input = Input::GetInstance();
	input->ShowMouseCursor(true);

	Object3dBase::GetInstance()->SetDefaultCamera(camera);

	object3d = new Object3d();
	object3d->Initialize();
	object3d->SetModel("terrain.obj");

	sprite = new Sprite();
	sprite->Initialize("Resources/uvChecker.png");

	cameraTransform.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform.rotate = { 0.36f, 0.0f, 0.0f };
	cameraTransform.translate = { 0.0f, 5.0f, -10.0f };

	modelTransform = object3d->GetTransform();
}

void GameScene::Update() {

	/*ImGui::Begin("State");
	if (ImGui::TreeNode("Camera")) {
		ImGui::DragFloat3("Tranlate", &cameraTransform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &cameraTransform.scale.x, 0.1f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Model")) {
		ImGui::DragFloat3("Tranlate", &modelTransform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate", &modelTransform.rotate.x, 0.1f);
		ImGui::DragFloat3("Scale", &modelTransform.scale.x, 0.1f);
		if (ImGui::TreeNode("AABB")) {
			ImGui::DragFloat3("Min", &aabb.min.x, 0.1f);
			ImGui::DragFloat3("Max", &aabb.max.x, 0.1f);

			ImGui::TreePop();
		}
		ImGui::Checkbox("EnableLihting", &enableLighting);
		ImGui::TreePop();
	}
	ImGui::End();*/

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
	if (input->TriggerKey(DIK_Q)) {
		ParticleManager::GetInstance()->Emit("slash", { 0.0f, 2.0f, 0.0f }, 10, ParticleStyle::Slash);
	}
	if (input->TriggerKey(DIK_E)) {
		ParticleManager::GetInstance()->Emit("slash", { 0.0f, 2.0f, 0.0f }, 20, ParticleStyle::HitEffect);
	}


	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
	camera->Update();
	object3d->SetTransform(modelTransform);
	object3d->SetEnableLighting(enableLighting);
	object3d->Update();
	aabb = object3d->GetAABB();
	sprite->Update();
	
	input->Update();

	ParticleManager::GetInstance()->Update();

}

void GameScene::Draw() {

	SpriteBase::GetInstance()->ShaderDraw();

	//sprite->Draw();

	Object3dBase::GetInstance()->ShaderDraw();

	object3d->Draw();

	ParticleManager::GetInstance()->Draw();


}

void GameScene::Finalize() {

	delete camera;

	delete object3d;

	delete sprite;

}