//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);
Texture2D normMap : register(t1);
Texture2D txQuad : register(t2);
Texture2D txBlur : register(t3);
Texture2D txDepth : register(t4);
Texture2D Albedo : register(t5);
Texture2D Specular : register(t6);
Texture2D Normal : register(t7);
Texture2D Position : register(t8);
Texture2D Lighting : register(t9);

#define MAX_LIGHTS 2
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _Material
{
	float4  Emissive;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Ambient;        // 16 bytes
							//------------------------------------(16 byte boundary)
	float4  Diffuse;        // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Specular;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float   SpecularPower;  // 4 bytes
	bool    UseTexture;
    bool UseNormalTexture; // 4 bytes
	float2  Padding;        // 8 bytes
							//----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b1)
{
	_Material Material;
};

struct Light
{
	float4      Position;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Direction;              // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Color;                  // 16 bytes
										//----------------------------------- (16 byte boundary)
	float       SpotAngle;              // 4 bytes
	float       ConstantAttenuation;    // 4 bytes
	float       LinearAttenuation;      // 4 bytes
	float       QuadraticAttenuation;   // 4 bytes
										//----------------------------------- (16 byte boundary)
	int         LightType;              // 4 bytes
	bool        Enabled;                // 4 bytes
	int2        Padding;                // 8 bytes
										//----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b2)
{
	float4 EyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4 GlobalAmbient;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
}; 

cbuffer PostProcessing : register(b3)
{
    bool GrayScale;
    bool Warm;
    float WarmAmount;
    bool Cold;
    float ColdAmount;
	
    bool DepthOfField;
    bool Bloom;
    bool GausianBlur;
    bool GausianH;
    bool GausianW;
    
    float temp;
    float temp1;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
    float4 EyePos : EYEPOSITION;
    float3x3 TBNMatrix : TBNMATRIX;
    float3 LightPosition :LIGHTPOSITION; 
    float3 vertexToEye : VERTEXTOEYE;
    float3 LightDirectionToVertex : LIGHTDIRECTIONTOVERTEX;
    float3 vertexToLight : VERTEXTOLIGHT;
    float distanceVTL : DISTANCEVTL;
    float distanceLTV : DISTANCELTV;
};

//struct VS_INPUT_G_BUFFER
//{
//    float2 Tex : TEXCOORD0;
//    float4 Pos : POSITION;
//    float3 Norm : NORMAL;
//    float3 Tangent : TANGENT;
//    float3 BiTangent : BITANGENT;
//};

struct PS_INPUT_G_BUFFER
{
    float4 Pos : SV_Position;
    float4 WorldPos : POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
    float3x3 TBNMatrix : TBNMATRIX;
};

struct PS_OUTPUT_G_BUFFER
{
    float4 Albedo : SV_Target0;
    float4 Specular : SV_Target1;
    float4 Normal : SV_Target2;
    float4 Position : SV_Target3;
};

//struct VS_INPUT_LIGHTING
//{
//    float2 Tex : TEXCOORD0;
//};

struct PS_INPUT_LIGHTING
{
    float2 Tex : TEXCOORD0;
    float4 Pos : SV_POSITION;
};

struct PS_OUTPUT_LIGHTING
{
    float4 Light : SV_TARGET0;
};

//struct VS_INPUT_FINAL
//{
//    float2 Tex : TEXCOORD0;
//};

struct PS_INPUT_FINAL
{
    float2 Tex : TEXCOORD0;
    float4 Pos : SV_POSITION;
};
struct PS_OUTPUT_FINAL
{
    float4 Colour : SV_TARGET0;

};

struct QuadVS_Input // note QuadVS_Input is the same
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct QuadVS_Output // note QuadVS_Input is the same
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 DoDiffuse(Light light, float3 L, float3 N)
{
	float NdotL = max(0, dot(N, L));
	return light.Color * NdotL;
}

float4 DoSpecular(Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal)
{
	float3 lightDir = -lightDirectionToVertex;

	float lightIntensity = saturate(dot(Normal, lightDir));
	float4 specular = float4(0, 0, 0, 0);
	if (lightIntensity > 0.0f)
	{
		//float3 reflection = normalize(2 * lightIntensity * Normal - lightDir);
        float3 reflection = reflect(-lightDir, Normal);
		specular = pow(saturate(dot(reflection, vertexToEye)), Material.SpecularPower); // 32 = specular power
	}
    
	return specular;
}

float4 DSDoSpecular(Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal, float4 SpP)
{
    float3 lightDir = -lightDirectionToVertex;

    float lightIntensity = saturate(dot(Normal, lightDir));
    float4 specular = float4(0, 0, 0, 0);
    if (lightIntensity > 0.0f)
    {
		//float3 reflection = normalize(2 * lightIntensity * Normal - lightDir);
        float3 reflection = reflect(-lightDir, Normal);
        specular = pow(saturate(dot(reflection, vertexToEye)), SpP.a); // 32 = specular power
    }
    
    return specular;
}

float DoAttenuation(Light light, float d)
{
	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

float DoSpotCone(Light light, float3 L)
{
    float minCos = cos(light.SpotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.Direction.xyz, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult DoPointLight(Light light, float3 vertexToLight, float3 vertexToEye, float distanceToLight, float3 N)
{
	LightingResult result;

 //   float3 LightDirectionToVertex = (vertexPos - float4(mul(light.Position.xyz, TBNMatrix), light.Position.w)).xyz;
	//float distance = length(LightDirectionToVertex);
	//LightDirectionToVertex = LightDirectionToVertex  / distance;

 //   float3 vertexToLight = (float4(mul(light.Position.xyz, TBNMatrix), light.Position.w) - vertexPos).xyz;
	//distance = length(vertexToLight);
	//vertexToLight = vertexToLight / distance;

    float attenuation = DoAttenuation(light, distanceToLight);
	//attenuation = 1;


	result.Diffuse = DoDiffuse(light, vertexToLight, N) * attenuation;
    result.Specular = DoSpecular(light, vertexToEye, -vertexToLight, N) * attenuation;

	return result;
}

LightingResult DoSpotLight(Light light, float3 V, float4 P, float3 N, float4 SpP)
{
    LightingResult result;
 
    float3 L = (light.Position - P).xyz;
    float distance = length(L);
    L = L / distance;
 
    float attenuation = DoAttenuation(light, distance);
    float spotIntensity = DoSpotCone(light, L);
 
    result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
    result.Specular = DSDoSpecular(light, V, L, N, SpP) * attenuation * spotIntensity;
 
    return result;
}

LightingResult DoDirectionalLight(Light light, float3 V, float4 P, float3 N, float4 SpP)
{
    LightingResult result;
 
    float3 L = -light.Direction.xyz;
 
    result.Diffuse = DoDiffuse(light, L, N);
    result.Specular = DSDoSpecular(light, V, L, N, SpP);
 
    return result;
}

LightingResult ComputeLighting(float3 vertexToLight, float3 vertexToEye, float distance, float3 N)
{

	LightingResult totalResult = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		LightingResult result = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

		if (!Lights[i].Enabled) 
			continue;
		
		result = DoPointLight(Lights[i], vertexToLight, vertexToEye, distance, N);
		
		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}

	totalResult.Diffuse = saturate(totalResult.Diffuse);
	totalResult.Specular = saturate(totalResult.Specular);

	return totalResult;
}

LightingResult OriginalDoPointLight(Light light, float3 V, float4 P, float3 N , float4 SpP)
{
    LightingResult result;
    
    float3 WorldLight = light.Position.xyz;
    
    float3 L = WorldLight - P.xyz;
    float distance = length(L);
    L = normalize(L / distance);
    
    float attenuation = DoAttenuation(light, distance);
    
    result.Diffuse = DoDiffuse(light, L, N) * attenuation;
    result.Specular = DSDoSpecular(light, V, L, N, SpP) * attenuation;
    return result;

}

LightingResult OriginalComputeLighting(float4 P, float3 N, float4 SpP)
{

    float3 V = normalize(normalize(mul(EyePosition, World)) - P).xyz;
    
    LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; ++i)
    {
        LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
        if (!Lights[i].Enabled)
            continue;
        switch (Lights[i].LightType)
        {
            case DIRECTIONAL_LIGHT:
            {
                    result = DoDirectionalLight(Lights[i], V, P, N, SpP);
                }
                break;
            case POINT_LIGHT:
            {
                    result = OriginalDoPointLight(Lights[i], V, P, N, SpP);
                }
                break;
            case SPOT_LIGHT:
            {
                    result = DoSpotLight(Lights[i], V, P, N, SpP);
                }
                break;
        }
        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }
    totalResult.Diffuse = saturate(totalResult.Diffuse);
    totalResult.Specular = saturate(totalResult.Specular);
    return totalResult;

}

PS_INPUT_G_BUFFER G_BUFFER_VS(VS_INPUT Input)
{
    PS_INPUT_G_BUFFER OGBUFFER;
    
    float3 normal = normalize(mul(Input.Norm, (float3x3) World));
    float3 tangent = normalize(mul(Input.Tangent, (float3x3) World));
    float3 bitangent = normalize(mul(Input.BiTangent, (float3x3) World));
	
    OGBUFFER.Pos = mul(Input.Pos, World);
    OGBUFFER.WorldPos = OGBUFFER.Pos;
    OGBUFFER.Pos = mul(OGBUFFER.Pos, View);
    OGBUFFER.Pos = mul(OGBUFFER.Pos, Projection);
    
    OGBUFFER.TBNMatrix = float3x3(tangent, bitangent, normal);
    
    OGBUFFER.Norm = Input.Norm;
    OGBUFFER.Tex = Input.Tex;
    return OGBUFFER;
}

PS_OUTPUT_G_BUFFER G_BUFFER_PS(PS_INPUT_G_BUFFER Input)
{
    PS_OUTPUT_G_BUFFER OGBUFFER;
    if (Material.UseNormalTexture)
    {
        float3 normalColour = normMap.Sample(samLinear, Input.Tex);
        normalColour = float3(normalColour.r * 2 - 1, normalColour.g * 2 - 1, normalColour.b * 2 - 1);
        float3 worldNormalColour = normalize(mul(normalColour, Input.TBNMatrix));
        OGBUFFER.Normal = float4(worldNormalColour, 1);
    }
    else
    {
        OGBUFFER.Normal = float4(normalize(mul(float4(Input.Norm, 0), World).xyz), 1);
    }
    
    OGBUFFER.Specular = float4(Material.Specular.xyz, Material.SpecularPower);
    
    OGBUFFER.Albedo = txDiffuse.Sample(samLinear, Input.Tex);
    OGBUFFER.Position = Input.WorldPos;
    return OGBUFFER;
}

PS_INPUT_LIGHTING LIGHTING_VS(QuadVS_Input Input)
{
    PS_INPUT_LIGHTING OLighting;
    OLighting.Tex = Input.Tex;
    OLighting.Pos = float4(Input.Pos, 1);
    return OLighting;
}

PS_OUTPUT_LIGHTING LIGHTING_PS(PS_INPUT_LIGHTING Input)
{
    PS_OUTPUT_LIGHTING OLighting;
    
    float3 NormalPixel = normalize(Normal.Sample(samLinear, Input.Tex).rgb);
    float4 SpecularPixel = Specular.Sample(samLinear, Input.Tex);
    float4 PositionPixel = Position.Sample(samLinear, Input.Tex);
    LightingResult result;
    result = OriginalComputeLighting(PositionPixel, NormalPixel, SpecularPixel);
    
    OLighting.Light = result.Diffuse + result.Specular * float4(SpecularPixel.xyz, 1);
    return OLighting;
}

PS_INPUT_FINAL FINAL_VS(QuadVS_Input Input)
{
    PS_INPUT_FINAL OFinal;
    OFinal.Tex = Input.Tex;
    OFinal.Pos = float4(Input.Pos, 1);
    return OFinal;
}

PS_OUTPUT_FINAL FINAL_PS(PS_INPUT_FINAL Input)
{
    PS_OUTPUT_FINAL OFinal;
    
    float4 AlbedoPixel = Albedo.Sample(samLinear, Input.Tex);
    float4 LightPixel = Lighting.Sample(samLinear, Input.Tex);
    
    OFinal.Colour = AlbedoPixel * LightPixel;
    return OFinal;
}

QuadVS_Output QuadVS(QuadVS_Input Input)
{
    QuadVS_Output Output;
    Output.Pos = float4(Input.Pos, 1.0f);
    Output.Tex = Input.Tex;
    return Output;
}


float4 QuadPS(QuadVS_Output Input) : SV_TARGET
{
    float4 vColor = txQuad.Sample(samLinear, Input.Tex);
	////https://support.ptc.com/help/mathcad/r10.0/en/index.html#page/PTC_Mathcad_Help/example_grayscale_and_color_in_images.html
	if(GrayScale)
    {
        float grey = 0.299 * vColor.r + 0.587 * vColor.g + 0.114 * vColor.b;
        vColor = float4(grey, grey, grey, 1);
    }
     //Warm
    else if (Warm)
    {
    vColor = float4(vColor.r * 1.2, vColor.g, vColor.b * 0.8, 1);
    }
	//Cold
    else if (Cold)
    {
    vColor = float4(vColor.r * 0.8, vColor.g, vColor.b * 1.2, 1);
    }
	if (GausianBlur)
    {
        vColor = 0;
        static const int SampleOffset[9] = { 4, 3, 2, 1, 0, -1, -2, -3, -4 };
        static const float SampleWeights[9] = { 0.000191f, 0.033871f, 0.0133f, 0.111f, 0.225f, 0.111f, 0.0133f, 0.033871f, 0.000191f };
        
        if (GausianW)
        {
            for (int i = 0; i < 9; i++)
            {
                vColor += txQuad.Sample(samLinear, float2(Input.Tex.x + float(SampleOffset[i]) / 480.0f, Input.Tex.y + 0)) * SampleWeights[i];
            }
        }
        else if(GausianH)
        {
            for (int i = 0; i < 9; i++)
            {
                vColor += txQuad.Sample(samLinear, float2(Input.Tex.x, Input.Tex.y + float(SampleOffset[i]) / 480.0f)) * SampleWeights[i];
            }
        }
    }
    else if (DepthOfField)
    {
        float4 vOriginalColor = txQuad.Sample(samLinear, Input.Tex);
        float4 vBlurredColor = txBlur.Sample(samLinear, Input.Tex);
        float fDepthVS = txDepth.Sample(samLinear, Input.Tex).x;
        vColor = vOriginalColor + (vBlurredColor - vOriginalColor) * (fDepthVS * 2 - 1);
        return vColor;
    }
    return vColor;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{   
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
	output.worldPos = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

	// multiply the normal by the world transform (to go from model space to world space)
	output.Norm = mul(float4(input.Norm, 0), World).xyz;

	output.Tex = input.Tex;
	
    float3 normal = normalize(mul(input.Norm, World));
    float3 tangent = normalize(mul(input.Tangent, World));
    float3 bitangent = normalize(mul(input.BiTangent, World));
	
    output.TBNMatrix = float3x3(tangent, bitangent, normal);
    output.TBNMatrix = transpose(output.TBNMatrix);
	
    output.LightPosition = mul(Lights[0].Position.xyz, output.TBNMatrix);
	
    output.EyePos = EyePosition;
    float3 EyePosTS = mul(EyePosition.xyz, output.TBNMatrix);
    float3 VertexPosTS = mul(output.worldPos.xyz, output.TBNMatrix);
	
    output.vertexToEye = normalize(EyePosTS - VertexPosTS).xyz;
   // output.LightDirectionToVertex = (normVertexPos - output.LightPosition);
    output.vertexToLight = output.LightPosition - VertexPosTS;
    output.distanceVTL = length(output.vertexToLight);
    //output.distanceLTV = length(output.LightDirectionToVertex);
    //output.LightDirectionToVertex = normalize(output.LightDirectionToVertex);
    output.vertexToLight = normalize(output.vertexToLight);
	
    return output;
    
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{
	// add vertex normal to normal colour Looks inverted
    float3 normalColour = normMap.Sample(samLinear, IN.Tex);
    normalColour = float3(normalColour.r * 2 - 1, normalColour.g * 2 - 1, normalColour.b * 2 - 1);
    //normalColour = normalize(mul(normalColour, IN.TBNMatrix));
    //normalColour = normalize(normalColour);
	
    //normalColour += IN.Norm;
    LightingResult lit = ComputeLighting(IN.vertexToLight, IN.vertexToEye, IN.distanceVTL, normalColour);

	float4 texColor = { 1, 1, 1, 1 };
    if (Material.UseTexture)
    {
        texColor = txDiffuse.Sample(samLinear, IN.Tex);
    }

	float4 emissive = Material.Emissive;
    float4 ambient = Material.Ambient * GlobalAmbient;
    float4 diffuse = Material.Diffuse * lit.Diffuse;
    float4 specular = Material.Specular * lit.Specular;

    float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

    return finalColor;
    }

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}
