#include "kMath.h"
#include <vector>
#include <map>
#include <string>
#include <cassert>

#pragma once

//template <typename tValue>
//struct Keyframe
//{
//    float time;
//    tValue value;
//};
//using KeyframeVector3 = Keyframe<Vector3>;
//using KeyframeQuaternion = Keyframe<Quaternion>;

//template<typename tValue>
//struct AnimationCurve
//{
//    std::vector < Keyframe<tValue> keyframes;
//};
//
//struct NodeAnimation
//{
//    AnimationCurve<Vector3> translate;
//    AnimationCurve<Quaternion> rotate;
//    AnimationCurve<Vector3> scale;
//};


struct KeyframeVector3
{
    Vector3 value;
    float time;
};

struct KeyframeQuaternion
{
    Quaternion value;
    float time;
};

struct NodeAnimation
{
    std::vector<KeyframeVector3> translate;
    std::vector<KeyframeQuaternion> rotate;
    std::vector<KeyframeVector3> scale;
};

struct Animation
{
    float duration; // アニメーション全体の尺(単位は秒)
    // NodeAnimationの場合、Node名を弾けるようにしておく
    std::map<std::string, NodeAnimation> nodeAnimations;
};

 inline Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
    assert(!keyframes.empty()); // キーが無いのは返す値が分からないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) // キーが一つか、時刻がキーフレーム前なら最初の秒とする
    {
        return keyframes[0].value;
    }
    
    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        // indexとnextIndexの二つのkeyframeを取得して範囲内に自国があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補完する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // ここまで来た場合は一番後の時刻うよりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
}

inline Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    assert(!keyframes.empty()); // キーが無いのは返す値が分からないのでダメ
    if (keyframes.size() == 1 || time <= keyframes[0].time) // キーが一つか、時刻がキーフレーム前なら最初の秒とする
    {
        return keyframes[0].value;
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        // indexとnextIndexの二つのkeyframeを取得して範囲内に自国があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 範囲内を補完する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    // ここまで来た場合は一番後の時刻うよりも後ろなので最後の値を返すことにする
    return (*keyframes.rbegin()).value;
}

