#include "Vector3.h"
#include "Quaternion.h"
#include <vector>
#include <map>
#include <assert.h>
#include "kMath.h"

#pragma once

#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

template <typename tValue>
struct KeyFrame {
	float time;
	tValue value;
};
using KeyFrameVector3 = KeyFrame<Vector3>;
using KeyFrameQuaternion = KeyFrame<Quaternion>;


template<typename tValue>
struct AnimationCurve {
	std::vector <KeyFrame<tValue>> KeyFrames;
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	float duration; // アニメーション全体の尺(単位は秒)
	// NodeAnimationの場合、Node名ひけるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimation;
	std::string nodeAnimationName;
};

inline Vector3 CalculateValue(const std::vector<KeyFrameVector3>& keyframes, float time) {
	assert(!keyframes.empty()); // キーが無いものは返す値が分からないのでassert
	if (keyframes.size() == 1 || time <= keyframes[0].time)// キーが1つか現在時刻がキーフレーム前なら最初の値とする
	{
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; index++)
	{
		size_t nextIndex = index + 1;
		// indexとnextIndexの二つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
		{
			// 範囲内をは保管する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}

inline Quaternion CalculateValue(const std::vector<KeyFrameQuaternion>& keyframes, float time) {
	// キーが無いものは返す必要がないのでデフォルト
	if (keyframes.empty())
	{
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	if (keyframes.size() == 1 || time <= keyframes[0].time)// キーが1つか現在時刻がキーフレーム前なら最初の値とする
	{
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index)
	{
		size_t nextIndex = index + 1;
		// indexとnextIndexの二つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
		{
			// 範囲内をは保管する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			ImGui::Begin("Quat");
			ImGui::DragFloat("t", &t, 0.0f);
			ImGui::End();
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);

		}
	}

	// ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}