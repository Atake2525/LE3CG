#include "MyGame.h"

void MyGame::Initialize() {

	FrameWork::Initialize();

#pragma region 基盤システムの初期化

	winApp = new WinApp();
	winApp->Initialize();

	directxBase = new DirectXBase();
	directxBase->Initialize(winApp);

	SpriteBase::GetInstance()->Initialize(directxBase);

	Object3dBase::GetInstance()->Initialize(directxBase);

	WireFrameObjectBase::GetInstance()->Initialize(directxBase);

	ModelBase::GetInstance()->Initialize(directxBase);

	TextureManager::GetInstance()->Initialize(directxBase);

	ModelManager::GetInstance()->Initialize(directxBase);

	Light::GetInstance()->Initialize(directxBase);

	Input::GetInstance()->Initialize(winApp);

	//// ↓---- シーンの初期化 ----↓ ////

	gameScene = new GameScene();

	gameScene->Initialize();

	//// ↑---- シーンの初期化 ----↑ ////
}

void MyGame::Update() {
	FrameWork::Update();

	if (winApp->ProcessMessage()) {
		finished = true;
	}
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	gameScene->Update();

	if (gameScene->isFinished())
	{
		finished = true;
	}
}

void MyGame::Draw() {

	// ImGuiの内部コマンドを生成する
	ImGui::Render();

	directxBase->PreDraw();

	gameScene->Draw();

	// 実際のcommandListのImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directxBase->GetCommandList().Get());

	directxBase->PostDraw();
}

void MyGame::Finalize() {

	winApp->Finalize();
	delete winApp;

	directxBase->Finalize();
	delete directxBase;

	SpriteBase::GetInstance()->Finalize();

	Object3dBase::GetInstance()->Finalize();

	WireFrameObjectBase::GetInstance()->Finalize();

	ModelBase::GetInstance()->Finalize();

	TextureManager::GetInstance()->Finalize();

	ModelManager::GetInstance()->Finalize();

	Light::GetInstance()->Finalize();

	Input::GetInstance()->Finalize();

	//// ↓---- シーンの解放 ----↓ ////

	gameScene->Finalize();
	delete gameScene;

	//// ↑---- シーンの解放 ----↑ ////

	FrameWork::Finalize();
}