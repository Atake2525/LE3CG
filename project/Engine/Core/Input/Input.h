#define DIRECTIONPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>
#include <Windows.h>
#include "Vector2.h"
#include "Vector3.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#pragma once

enum class DPad {
	None,
	Up,
	UpRight,
	UpLeft,
	Down,
	DownRight,
	DownLeft,
	Left,
	Right,
};

enum class Button {
	None,
	A,
	B,
	X,
	Y,
	LB,
	RB,
	LT,
	RT,
	View,
	Menu,
	LeftStick,
	RightStick,
};

class WinApp;

class Input {
private:
	// シングルトンパターンを適用
	static Input* instance;

	// コンストラクタ、デストラクタの隠蔽
	Input() = default;
	~Input() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	//Input(Input&) = delete;
	//Input& operator=(Input&) = delete;

public:
	// namespcae省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>Input* instance</returns>
	static Input* GetInstance();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	void Initialize(WinApp* winApp);
	void Update();

	// デバイスの更新(デバイスの再認識に使う)
	void UpdateDevice();

public:
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>押されているか</returns>
	const bool& PushKey(BYTE keyNumber) const;

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>押したかどうか</returns>
	const bool& TriggerKey(BYTE keyNumber) const;


	/// <summary>
	/// キーのリターンをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>離されたか</returns>
	const bool& ReturnKey(BYTE keyNumber) const;

	/// <summary>
	/// マウスの押下をチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左ボタン</param>
	/// <param name="mouseNumer">1 = 右ボタン</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	const bool& PressMouse(int mouseNumer) const;

	/// <summary>
	/// マウスのトリガーをチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左ボタン</param>
	/// <param name="mouseNumer">1 = 右ボタン</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	const bool& TriggerMouse(int mouseNumber) const;

	/// <summary>
	/// マウスのリターンをチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左ボタン</param>
	/// <param name="mouseNumer">1 = 右ボタン</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	const bool& ReturnMouse(int mouseNumber) const;

	// マウスの移動量を取得(Vector2)
	const Vector2& GetMouseVel2() const;

	// マウスの移動量を取得(Vector3)
	const Vector3& GetMouseVel3() const;

	/// <summary>
	/// マウスカーソルの表示変更
	/// </summary>
	/// <param name="">True  = 表示</param>
	/// <param name="">False = 非表示</param>
	void ShowMouseCursor(bool flag);

	// ジョイスティック左の傾きを取得(Vector2)
	const Vector2& GetLeftJoyStickPos2() const;

	// ジョイスティック左の傾きを取得(Vector3)
	const Vector3& GetLeftJoyStickPos3() const;

	// ジョイスティック右の傾きを取得(Vector2)
	const Vector2& GetRightJoyStickPos2() const;

	// ジョイスティック右の傾きを取得(Vector3)
	const Vector3& GetRightJoyStickPos3() const;

	const bool& IsMoveLeftJoyStick() const;

	const bool& IsMoveRightJoyStick() const;


	/// <summary>
	/// 十字キー(コントローラー)の押下をチェック
	/// </summary>
	const bool& PushXButton(DPad dPad) const;

	/// <summary>
	/// 十字キー(コントローラー)のトリガーをチェック
	/// </summary>
	const bool& TriggerXButton(DPad dPad) const;

	/// <summary>
	/// ボタン(コントローラー)の押下をチェック
	/// </summary>
	const bool& PushButton(Button button) const;

	/// <summary>
	/// ボタン(コントローラー)のトリガーをチェック
	/// </summary>
	const bool& TriggerButton(Button button) const;

	/// <summary>
	/// ボタン(コントローラー)のリターンをチェック
	/// </summary>
	const bool& ReturnButton(Button button) const;

private:

	// DirectInputのインスタンス生成 キーボード
	ComPtr<IDirectInput8> directInput = nullptr;
	// DorectxInputのインスタンス生成 マウス
	ComPtr<IDirectInput8> directInputMouse = nullptr;
	// DorectxInputのインスタンス生成 コントローラー(ゲームパッド)
	ComPtr<IDirectInput8> directInputGamePad = nullptr;

	void CreateKeyboardDevice();
	void CreateMouseDevice();
	void CreateControllerDevice();

	// キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboard;

	WinApp* winApp_ = nullptr;

	// 全キーの状態
	BYTE keys[256] = {};
	// 前回の全キーの状態
	BYTE keyPres[256] = {};

	// マウスデバイス
	ComPtr<IDirectInputDevice8> mouse;

	// 全マウスの状態
	DIMOUSESTATE mouseState = {};
	// 前回の全マウスの状態
	DIMOUSESTATE mouseStatePre = {};
	// マウスカーソル表示
	bool showCursor = false;

	// コントローラーデバイス
	ComPtr<IDirectInputDevice8> gamePad;

	// コントローラーが接続されているか
	bool isControllerConnected = false;

	// スティックの無効範囲
	double unresponsiveRange = 100;

	// コントローラーの状態
	DIJOYSTATE gamePadState;
	// 前回のコントローラーの状態
	DIJOYSTATE gamePadStatePre;

	// 軸モードを絶対値モードとして設定



};
