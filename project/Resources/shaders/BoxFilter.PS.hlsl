#include "Vignetting.PS.hlsl"
    
static const float PI = 3.14159265f;

struct BoxFilter
{
    bool enableBoxFilter;
    int size;
};
ConstantBuffer<BoxFilter> gBoxFilter : register(b2);

PixelShaderOutput main(VertexShaderOutput input)
{
    
    
    PixelShaderOutput output;
    output = ShadingVignetting(input);
    
    if (gBoxFilter.enableBoxFilter)
    {
    
        const float kernel = 1.0f / (gBoxFilter.size * gBoxFilter.size);
        
        uint32_t width, height; // 1. uvStepSize‚ÌZo
        gTexture.GetDimensions(width, height);
        float2 uvStepSize = float2(rcp(width), rcp(height));
        float3 color = float3(0.0f, 0.0f, 0.0f);
        
        // index‚Ì’†S‚ğZo
        int center = gBoxFilter.size - ((gBoxFilter.size - 1) / 2);
        for (int32_t x = 0; x < gBoxFilter.size + 0; ++x)
        {
            for (int32_t y = 0; y < gBoxFilter.size + 0; ++y)
            {
            // 3. Œ»İ‚Ìtexcoord‚ğZo
                float2 texcoord = input.texcoord + float2(x - center, y - center) * uvStepSize;
            // 4. F‚É1/9Š|‚¯‚Ä‘«‚·
                float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
                color.rgb += fetchColor * kernel;
            }
        }
    
        output.color.rgb = color;
    }
    
    return output;
}