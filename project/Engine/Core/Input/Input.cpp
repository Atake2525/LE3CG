#include "Input.h"
#include <cassert>
#include "WinApp.h"

Input* Input::instance = nullptr;

Input* Input::GetInstance() {
	if (instance == nullptr) {
		instance = new Input;
	}
	return instance;
}

void Input::Finalize() {
	delete instance;
	instance = nullptr;
}

void Input::Initialize(WinApp* winApp) {
	winApp_ = winApp;
	HRESULT result;

	// DirectInputのインスタンス生成 キーボード
	result = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	result = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInputMouse, nullptr);
	assert(SUCCEEDED(result));

	result = DirectInput8Create(winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInputGamePad, nullptr);
	assert(SUCCEEDED(result));

	CreateKeyboardDevice();
	CreateMouseDevice();
	CreateControllerDevice();

}

void Input::UpdateDevice() {
	CreateKeyboardDevice();
	CreateMouseDevice();
	CreateControllerDevice();
}

void Input::CreateKeyboardDevice() {
	HRESULT result;
	// DirectInputの初期化
	// マウスデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	// 入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	// 排他制御レベルセット
	result = keyboard->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::CreateMouseDevice() {
	HRESULT result;

	// キーボードデバイスの生成
	result = directInputMouse->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(result));
	// 入力データ形式のセット
	result = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	// 排他制御レベルセット
	result = mouse->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	assert(SUCCEEDED(result));
	// 排他制御レベルセット
	result = mouse->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	assert(SUCCEEDED(result));
}

void Input::CreateControllerDevice() {
	HRESULT result;


	// ゲームパッドデバイスの生成
	result = directInputGamePad->CreateDevice(GUID_Joystick, &gamePad, NULL);
	if (FAILED(result))
	{
		isControllerConnected = false;
	}
	else
	{
		isControllerConnected = true;
	}
	if (isControllerConnected)
	{
		// 入力データ形式のセット
		result = gamePad->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));

		// 軸モード設定
		DIPROPDWORD diprop;
		ZeroMemory(&diprop, sizeof(diprop));
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.diph.dwObj = 0;
		diprop.dwData = DIPROPAXISMODE_ABS; // 絶対値モードの指定(DIPROPAXISMODE_RELにしたら相対値)

		// 軸モードを変更
		result = gamePad->SetProperty(DIPROP_AXISMODE, &diprop.diph);

		// X軸の値の範囲設定
		DIPROPRANGE diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.diph.dwObj = DIJOFS_X;
		diprg.lMin = -1000;
		diprg.lMax = 1000;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// Y軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_Y;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// Y軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_Z;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// RX軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_RX;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// RY軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_RY;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);
	}
}

void Input::ShowMouseCursor(bool flag) {
	if (flag)
	{
		mouse->Unacquire();
	}
	showCursor = flag;
}

void Input::Update() {
	HRESULT result;

	// 前回のキー入力を保存
	memcpy(keyPres, keys, sizeof(keys));
	// キーボード情報の取得開始
	result = keyboard->Acquire();
	// 全キーの入力情報を取得する
	result = keyboard->GetDeviceState(sizeof(keys), keys);

	if (!showCursor)
	{
		mouseStatePre = mouseState;
		// マウスの状態の取得
		result = mouse->Acquire();
		// ポーリング開始
		result = mouse->Poll();
		// 全ボタンの入力情報を取得する
		result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
	}

	if (isControllerConnected)
	{
		// 前回のコントローラーの入力を保存
		gamePadStatePre = gamePadState;
		// コントローラーの状態取得開始
		result = gamePad->Acquire();
		// ポーリング開始
		result = gamePad->Poll();
		// 入力情報を取得
		result = gamePad->GetDeviceState(sizeof(DIJOYSTATE), &gamePadState);
	}

}

const bool& Input::PushKey(BYTE keyNumber) const {
	if (keys[keyNumber]) {
		return true;
	}
	return false;
}

const bool& Input::TriggerKey(BYTE keyNumber) const {
	if (keys[keyNumber] && !keyPres[keyNumber]) {
		return true;
	}
	return false;
}

const bool& Input::ReturnKey(BYTE keyNumber) const {
	if (!keys[keyNumber] && keyPres[keyNumber]) {
		return true;
	}
	return false;
}

const bool& Input::PressMouse(int mouseNumber) const {
	if (mouseState.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

const bool& Input::TriggerMouse(int mouseNumber) const {
	if (mouseState.rgbButtons[mouseNumber] && !mouseStatePre.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

const bool& Input::ReturnMouse(int mouseNumber) const {
	if (!mouseState.rgbButtons[mouseNumber] && mouseStatePre.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

const Vector2& Input::GetMouseVel2() const {
	Vector2 result = { static_cast<float>(mouseState.lX), static_cast<float>(mouseState.lY) };
	return result;
}

const Vector3& Input::GetMouseVel3() const {
	Vector3 result = { static_cast<float>(mouseState.lX), static_cast<float>(mouseState.lY), static_cast<float>(mouseState.lZ) };
	return result;
}

const Vector2& Input::GetLeftJoyStickPos2() const {
	Vector2 result = { 0.0f, 0.0f };
	if (gamePadState.lX < -unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lX);
	}
	else if (gamePadState.lX > unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lX);
	}

	if (gamePadState.lY < -unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lY);
	}
	else if (gamePadState.lY > unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lY);
	}
	return result;
}

const Vector3& Input::GetLeftJoyStickPos3() const {
	Vector3 result = { 0.0f, 0.0f, 0.0f };
	if (gamePadState.lX < -unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lX);
	}
	else if (gamePadState.lX > unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lX);
	}

	if (gamePadState.lY < -unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lY);
	} 
	else if (gamePadState.lY > unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lY);
	}

	if (gamePadState.lZ < -unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lZ);
	}
	else if (gamePadState.lZ > unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lZ);
	}
	return result;
}

const Vector2& Input::GetRightJoyStickPos2() const {
	Vector2 result = { 0.0f, 0.0f };
	if (gamePadState.lRx < -unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}
	else if (gamePadState.lRx > unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}

	if (gamePadState.lRy < -unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	else if (gamePadState.lRy > unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	return result;
}

const Vector3& Input::GetRightJoyStickPos3() const {
	Vector3 result = { 0.0f, 0.0f, 0.0f };
	if (gamePadState.lRx < -unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}
	else if (gamePadState.lRx > unresponsiveRange)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}

	if (gamePadState.lRy < -unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	else if (gamePadState.lRy > unresponsiveRange)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}

	if (gamePadState.lRz < -unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lRz);
	}
	else if (gamePadState.lRz > unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lRz);
	}
	return result;
}

const bool& Input::IsMoveLeftJoyStick() const{
	if (gamePadState.lX < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lX > unresponsiveRange)
	{
		return true;
	}

	if (gamePadState.lY < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lY > unresponsiveRange)
	{
		return true;
	}
	//if (gamePadState.lX != 0 || gamePadState.lY != 0)
	//{
	//	return true;
	//}
	return false;
}

const bool& Input::IsMoveRightJoyStick() const {
	if (gamePadState.lRx < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lRx > unresponsiveRange)
	{
		return true;
	}

	if (gamePadState.lRy < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lRy > unresponsiveRange)
	{
		return true;
	}
	//if (gamePadState.lRx != 0 || gamePadState.lRy != 0)
	//{
	//	return true;
	//}
	return false;
}

const bool& Input::PushXButton(DPad dPad) const {
	DPad result = DPad::None;
	switch (gamePadState.rgdwPOV[0])
	{
	case 0: // 上
		result = DPad::Up;
		break;

	case 18000: // 下
		result = DPad::Down;
		break;

	case 9000: // 右
		result = DPad::Right;
		break;

	case 27000: // 左
		result = DPad::Left;
		break;

	case 4500: // 右上
		result = DPad::UpRight;
		break;
		
	case 31500: // 左上
		result = DPad::UpLeft;
		break;

	case 13500: // 右下
		result = DPad::DownRight;
		break;

	case 22500: // 左下
		result = DPad::DownLeft;
		break;

	}


	if (dPad == result)
	{
		return true;
	}
	return false;
}

const bool& Input::TriggerXButton(DPad dPad) const {
	DPad result = DPad::None;
	switch (gamePadState.rgdwPOV[0])
	{
	case 0: // 上
		result = DPad::Up;
		break;

	case 18000: // 下
		result = DPad::Down;
		break;

	case 9000: // 右
		result = DPad::Right;
		break;

	case 27000: // 左
		result = DPad::Left;
		break;

	case 4500: // 右上
		result = DPad::UpRight;
		break;

	case 31500: // 左上
		result = DPad::UpLeft;
		break;

	case 13500: // 右下
		result = DPad::DownRight;
		break;

	case 22500: // 左下
		result = DPad::DownLeft;
		break;

	}

	DPad resultPre = DPad::None;

	switch (gamePadStatePre.rgdwPOV[0])
	{
	case 0: // 上
		resultPre = DPad::Up;
		break;

	case 18000: // 下
		resultPre = DPad::Down;
		break;

	case 9000: // 右
		resultPre = DPad::Right;
		break;

	case 27000: // 左
		resultPre = DPad::Left;
		break;

	case 4500: // 右上
		resultPre = DPad::UpRight;
		break;

	case 31500: // 左上
		resultPre = DPad::UpLeft;
		break;

	case 13500: // 右下
		resultPre = DPad::DownRight;
		break;

	case 22500: // 左下
		resultPre = DPad::DownLeft;
		break;

	}


	if (dPad == result && dPad != resultPre)
	{
		return true;
	}
	return false;
}

const bool& Input::PushButton(Button button) const {
	Button result = Button::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Button::A;
			if (button == result)
			{
				return true;
				break;
			}
		case 1:
			result = Button::B;
			if (button == result)
			{
				return true;
			}
		case 2:
			result = Button::X;
			if (button == result)
			{
				return true;
			}
		case 3:
			result = Button::Y;
			if (button == result)
			{
				return true;
			}
		case 4:
			result = Button::LB;
			if (button == result)
			{
				return true;
			}
		case 5:
			result = Button::RB;
			if (button == result)
			{
				return true;
			}
		case 6:
			result = Button::View;
			if (button == result)
			{
				return true;
			}
		case 7:
			result = Button::Menu;
			if (button == result)
			{
				return true;
			}
		case 8:
			result = Button::LeftStick;
			if (button == result)
			{
				return true;
			}
		case 9:
			result = Button::RightStick;
			if (button == result)
			{
				return true;
			}
		}
	}
	Vector3 joystick = GetLeftJoyStickPos3();
	if (joystick.z < 0)
	{
		result = Button::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z > 0)
	{
		result = Button::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}

const bool& Input::TriggerButton(Button button) const {
	Button result = Button::None;
	Button resultPre = Button::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Button::A;
			if (button == result)
			{
				break;
			}
		case 1:
			result = Button::B;
			if (button == result)
			{
				break;
			}
		case 2:
			result = Button::X;
			if (button == result)
			{
				break;
			}
		case 3:
			result = Button::Y;
			if (button == result)
			{
				break;
			}
		case 4:
			result = Button::LB;
			if (button == result)
			{
				break;
			}
		case 5:
			result = Button::RB;
			if (button == result)
			{
				break;
			}
		case 6:
			result = Button::View;
			if (button == result)
			{
				break;
			}
		case 7:
			result = Button::Menu;
			if (button == result)
			{
				break;
			}
		case 8:
			result = Button::LeftStick;
			if (button == result)
			{
				break;
			}
		case 9:
			result = Button::RightStick;
			if (button == result)
			{
				break;
			}
		}
	}

	for (int i = 0; i < 10; i++)
	{
		if (!gamePadStatePre.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			resultPre = Button::A;
			if (button == resultPre)
			{
				break;
			}
		case 1:
			resultPre = Button::B;
			if (button == resultPre)
			{
				break;
			}
		case 2:
			resultPre = Button::X;
			if (button == resultPre)
			{
				break;
			}
		case 3:
			resultPre = Button::Y;
			if (button == resultPre)
			{
				break;
			}
		case 4:
			resultPre = Button::LB;
			if (button == resultPre)
			{
				break;
			}
		case 5:
			resultPre = Button::RB;
			if (button == resultPre)
			{
				break;
			}
		case 6:
			resultPre = Button::View;
			if (button == resultPre)
			{
				break;
			}
		case 7:
			resultPre = Button::Menu;
			if (button == resultPre)
			{
				break;
			}
		case 8:
			resultPre = Button::LeftStick;
			if (button == resultPre)
			{
				break;
			}
		case 9:
			resultPre = Button::RightStick;
			if (button == resultPre)
			{
				break;
			}
		}
	}

	if (result == button && resultPre != button)
	{
		return true;
	}

	Vector3 joystick = GetLeftJoyStickPos3();
	float joystickPre = 0.0f;
	if (gamePadStatePre.lZ < -unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}
	else if (gamePadStatePre.lZ > unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}

	if (joystick.z < 0 && joystickPre >= 0)
	{
		result = Button::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z > 0 && joystickPre <= 0)
	{
		result = Button::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}

const bool& Input::ReturnButton(Button button) const {
	Button result = Button::None;
	Button resultPre = Button::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Button::A;
			if (button == result)
			{
				break;
			}
		case 1:
			result = Button::B;
			if (button == result)
			{
				break;
			}
		case 2:
			result = Button::X;
			if (button == result)
			{
				break;
			}
		case 3:
			result = Button::Y;
			if (button == result)
			{
				break;
			}
		case 4:
			result = Button::LB;
			if (button == result)
			{
				break;
			}
		case 5:
			result = Button::RB;
			if (button == result)
			{
				break;
			}
		case 6:
			result = Button::View;
			if (button == result)
			{
				break;
			}
		case 7:
			result = Button::Menu;
			if (button == result)
			{
				break;
			}
		case 8:
			result = Button::LeftStick;
			if (button == result)
			{
				break;
			}
		case 9:
			result = Button::RightStick;
			if (button == result)
			{
				break;
			}
		}
	}

	for (int i = 0; i < 10; i++)
	{
		if (!gamePadStatePre.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			resultPre = Button::A;
			if (button == resultPre)
			{
				break;
			}
		case 1:
			resultPre = Button::B;
			if (button == resultPre)
			{
				break;
			}
		case 2:
			resultPre = Button::X;
			if (button == resultPre)
			{
				break;
			}
		case 3:
			resultPre = Button::Y;
			if (button == resultPre)
			{
				break;
			}
		case 4:
			resultPre = Button::LB;
			if (button == resultPre)
			{
				break;
			}
		case 5:
			resultPre = Button::RB;
			if (button == resultPre)
			{
				break;
			}
		case 6:
			resultPre = Button::View;
			if (button == resultPre)
			{
				break;
			}
		case 7:
			resultPre = Button::Menu;
			if (button == resultPre)
			{
				break;
			}
		case 8:
			resultPre = Button::LeftStick;
			if (button == resultPre)
			{
				break;
			}
		case 9:
			resultPre = Button::RightStick;
			if (button == resultPre)
			{
				break;
			}
		}
	}

	if (result != button && resultPre == button)
	{
		return true;
	}

	Vector3 joystick = GetLeftJoyStickPos3();
	float joystickPre = 0.0f;
	if (gamePadStatePre.lZ < -unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}
	else if (gamePadStatePre.lZ > unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}

	if (joystick.z >= 0 && joystickPre < 0)
	{
		result = Button::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z <= 0 && joystickPre > 0)
	{
		result = Button::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}