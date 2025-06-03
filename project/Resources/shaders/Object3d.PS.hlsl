#include "object3d.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

/*struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};*/

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
    float32_t3 specularColor;
};
ConstantBuffer<Material> gMaterial : register(b0);
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

struct Camera
{
    float32_t3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b1);

struct DirectionalLight
{
    float32_t4 color; //!< ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity; //!< 輝度
    float32_t3 specularColor;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b2);

struct PointLight {
    float32_t4 color; //!< ライトの色
    float32_t3 position; //!< ライトの位置
    float intensity; //!< 輝度
    float radius; //!< ライトの届く最大距離
    float dacay; //!< 減衰率
    float32_t3 specularColor;
};
//ConstantBuffer<PointLight> gPointLight : register(b3);

//int pointLightCount = 10;

struct PLight {
    PointLight light[10];
    int lightCount;
};
ConstantBuffer<PLight> gPointLights : register(b3);

struct SpotLight {
    float32_t4 color; //!< ライトの色
    float32_t3 position; //!< ライトの位置
    float32_t intensity; //!< 輝度
    float32_t3 direction; //!< スポットライトの方向
    float32_t distance; //!< ライトの届く最大距離
    float32_t dacay; //!< 減衰率
    float32_t cosAngle; //!< スポットライトの余弦
    float32_t cosFalloffStart; // falloffが開始される角度
    float32_t3 specularColor;
};
ConstantBuffer<SpotLight> gSpotLight : register(b4);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (gMaterial.enableLighting != 0)
    { // Lightingする場合
        // Half lambert
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        // Phong Reflection Model
        // 計算式 R = reflect(L,N) specular = (V.R)n
        float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        //float32_t3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
    
        // HalfVectorを求めて計算する
        float32_t3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        
        //float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess); // 反射強度
        
        
        //float NdotLPointLight = dot(normalize(input.normal), -pointLightDirection);
        //float cosPointLight = pow(NdotLPointLight * 0.5f + 0.5f, 2.0f);
        
        // spotLight
        float32_t3 spotLightDirectionOnSurFace = normalize(input.worldPosition - gSpotLight.position);
        
        float32_t cosAngle = dot(spotLightDirectionOnSurFace, gSpotLight.direction);
        float32_t falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        
        float NdotLSpotLight = dot(normalize(input.normal), -spotLightDirectionOnSurFace);
        float cosSpotLight = pow(NdotLSpotLight * 0.5f + 0.5f, 2.0f);
        
        //float32_t attenuationFactor = saturate((cosAngle - gSpotLight.distance) / (1.0f - gSpotLight.distance));
        
        float32_t spotLightdistance = length(gSpotLight.position - input.worldPosition); // ポイントライトへの距離
        float32_t attenuationFactor = pow(saturate(-spotLightdistance / gSpotLight.distance + 1.0f), gSpotLight.dacay); // 逆に上による減衰係数
        
        // DirectionalLight
        // 拡散反射
        float32_t3 diffuseDirectionalLight = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        float32_t3 specularDirectionalLight = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gDirectionalLight.specularColor;
        
        float32_t3 diffusePLight = { 0.0f, 0.0f, 0.0f };
        float32_t3 specularPLight = { 0.0f, 0.0f, 0.0f };
        for (int i = 0; i < min(gPointLights.lightCount, 10); i++) {
             // pointLight
            float32_t3 pointLightDirection = normalize(input.worldPosition - gPointLights.light[i].position);
            float32_t3 halfVectorPointLight = normalize(-pointLightDirection + toEye);
            float NDotHPointLight = dot(normalize(input.normal), halfVectorPointLight);
        
            float specularPowPointLight = pow(saturate(NDotHPointLight), gMaterial.shininess);
        
            float32_t distance = length(gPointLights.light[i].position - input.worldPosition); // ポイントライトへの距離
            float32_t factor = pow(saturate(-distance / gPointLights.light[i].radius + 1.0f), gPointLights.light[i].dacay); // 逆に上による減衰係数
        
            // PointLight
            // 拡散反射
            float32_t3 diffusePointLight = gMaterial.color.rgb * textureColor.rgb * gPointLights.light[i].color.rgb * cos * gPointLights.light[i].intensity * factor;
        
            diffusePLight += diffusePointLight;
            
            // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
            float32_t3 specularPointLight = gPointLights.light[i].color.rgb * gPointLights.light[i].intensity * factor * specularPowPointLight * gPointLights.light[i].specularColor.rgb;
            
            specularPLight += specularPointLight;
        }
        
        // SpotLight
         // 拡散反射
            float32_t3 diffuseSpotLight = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * cosSpotLight * gSpotLight.intensity * falloffFactor * attenuationFactor;
        
        // 鏡面反射                                                                                      ↓ 物体の鏡面反射の色。ここでは白にしている materialで設定できたりすると良い
        float32_t3 specularSpotLight = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor * gSpotLight.specularColor * specularPow;
        
        // 拡散反射 + 鏡面反射
        output.color.rgb = diffuseDirectionalLight + specularDirectionalLight + diffusePLight + specularPLight + diffuseSpotLight + specularSpotLight;
        // アルファは今まで通り
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    { // Lightingしない場合。前回までと同じ計算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}