#pragma once

#include <string>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>

class Shader
{
public:
	// コンストラクタ
	Shader();
	// デストラクタ
	~Shader();


	// 生成したコンパイル済データを取得する
	Microsoft::WRL::ComPtr<IDxcBlob> GetBlob();

	// シェーダーファイルを読み込み、コンパイル済データを生成する
	void Load(const std::wstring& filePath, const wchar_t* profile);

private:
	Microsoft::WRL::ComPtr<IDxcBlob> blob_ = nullptr;

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
};