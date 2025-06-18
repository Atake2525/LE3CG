#include "Grayscale.PS.hlsl"

struct Vignette
{
    int enableVignette;
    float intensity; //!< ‹P“x
    float scale;
};
ConstantBuffer<Vignette> gVigentte : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output = ShadingGrayscale(input); 
    //output.color = gTexture.Sample(gSampler, input.texcoord);
    
    if (gVigentte.enableVignette)
    {
         // üˆÍ‚ğ0‚ÉA’†S‚É‚È‚é‚Ù‚Ç–¾‚é‚­‚È‚é—l‚ÉŒvZ‚Å’²®
        float2 correct = input.texcoord * (1.0f - input.texcoord.yx * 1.0f);
        // correct‚¾‚¯‚ÅŒvZ‚·‚é‚Æ’†S‚ÌÅ‘å’l‚ª0.0625‚ÅˆÃ‚·‚¬‚é‚Ì‚Åscale‚Å’²®B‚±‚Ì—á‚Å‚Í16”{‚µ‚Ä‚P‚É‚µ‚Ä‚¢‚é
        float vignette = correct.x * correct.y * gVigentte.intensity;
        // ‚Æ‚è‚ ‚¦‚¸0.8æ‚Å‚»‚ê‚Á‚Û‚­‚µ‚Ä‚İ‚½
        vignette = saturate(pow(vignette, gVigentte.scale));
        // ŒW”‚Æ‚µ‚ÄæZ
        output.color.rgb *= vignette;
    }
    
    return output;
    
}

