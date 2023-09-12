import Scene.Raster;

struct VsOut
{
    float2 texC : TEXCOORD;
    float4 posH : SV_POSITION;
};

Texture2D<float4> gPosW;
Texture2D<float4> gNormalW;
Texture2D<float2> gTexC;
Texture2D<uint4>  gMaterialData;

float4 main(VsOut vsOut, uint triangleIndex: SV_PrimitiveID) : SV_TARGET0
{
    float4 psOut;

    uint3 screenC = uint3(uint(vsOut.posH.x), uint(vsOut.posH.y), 0);
    MaterialSystem ms = gScene.materials;
    
    // let lod = ImplicitLodTextureSampler();
    // SamplerState sampler = ms.getDefaultTextureSampler(vsOut.materialID);

    float4 posW = gPosW.Load(screenC);
    psOut = posW;
    
    return psOut;
}
