#include "Vignetting.PS.hlsl"

static const float kIndex3x3[3][3] =
{
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
    { 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
};
    
static const float PI = 3.14159265f;

struct BoxFilter
{
    bool enableBoxFilter;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    
    uint32_t width, height; // 1. uvStepSizeÇÃéZèo
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(width), rcp(height));
    
    PixelShaderOutput output;
    output = ShadingVignetting(input);
    
    float3 color = float3(0.0f, 0.0f, 0.0f);
    //float alpha = 1.0f;
    
    for (int32_t x = 0; x < 3; ++x) {
        for (int32_t y = 0; y < 3; ++y) {
            // 3. åªç›ÇÃtexcoordÇéZèo
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStepSize;
            // 4. êFÇ…1/9ä|ÇØÇƒë´Ç∑
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            color.rgb += fetchColor * kIndex3x3[x][y];
        }
    }
    
    output.color.rgb = color;
    
    return output;
}