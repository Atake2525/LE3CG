#include "Fullscreen.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct Monotone
{
    float32_t3 monotone;
};
ConstantBuffer<Monotone> gMonotone : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    float32_t value = dot(output.color.rbg, float32_t3(0.2125f, 0.7154f, 0.0721f));
    // モノトーンのRGBの中で最も大きい数値を抽出
    float baseTone = max(gMonotone.monotone.x, gMonotone.monotone.y);
    baseTone = max(baseTone, gMonotone.monotone.z);
    output.color.rgb = value * float32_t3(gMonotone.monotone.x / baseTone, gMonotone.monotone.y / baseTone, gMonotone.monotone.z / baseTone);
    return output;
}