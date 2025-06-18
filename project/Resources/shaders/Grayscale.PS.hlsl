#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Monotone
{
    float3 toneColor;
};
ConstantBuffer<Monotone> gMonotone : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    float value = dot(output.color.rbg, float3(0.2125f, 0.7154f, 0.0721f));
    //float32_t value = dot(output.color.rbg, float32_t3(1.0f, 1.0f, 1.0f));
    // モノトーンのRGBの中で最も大きい数値を抽出
    //float baseTone = max(gMonotone.toneColor.x, gMonotone.toneColor.y);
    //baseTone = max(baseTone, gMonotone.toneColor.z);
    //output.color.rgb = value * float32_t3(gMonotone.x / baseTone, gMonotone.y / baseTone, gMonotone.z / baseTone);
    output.color.rgb = value * float3(gMonotone.toneColor.x, gMonotone.toneColor.y, gMonotone.toneColor.z);
    //output.color.rgb = value * float32_t3(1.0f, 73.0f / 107.0f, 43.0f / 107.0f);
    return output;
}