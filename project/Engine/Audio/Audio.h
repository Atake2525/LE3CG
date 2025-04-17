#include <xaudio2.h>
#include <fstream>
#include <wrl.h>
#include <vector>

#pragma once

#pragma comment(lib, "xaudio2.lib")

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
	// ファイルの名前
	const char* filename;
	// ファイルの再生時間
	int playTime;
};

struct AudioList
{
	IXAudio2SourceVoice* sourceVoice;
	SoundData soundData;
	int startFrameTime;
};

class Audio {
private:
	// シングルトンパターンを適用
	static Audio* instance;

	// コンストラクタ、デストラクタの隠蔽
	Audio() = default;
	~Audio() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;

public:

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// インスタンスの取得
	static Audio* GetInstance();

	// 終了処理
	void Finalize();

	// 音声読み込み
	SoundData SoundLoadWave(const char* filename);

	// 音声再生
	void SoundPlayWave(const SoundData& soundData, float volume);

	// 全ての音声停止
	void SoundStopWaveAll();

	// 音声停止
	void SoundStopWave(const SoundData& soundData);

	// 音声データ解放
	void SoundUnload(SoundData* soundData);

private:

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t maxSourceVoiceCount;

	// audio test
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice = nullptr;

	std::vector<AudioList> audioList;

	int frameTime = 0;
};