import ShaderLib.Common;

#define is_valid(name) (is_valid_##name != 0)

// GBuffer channels
struct GBufferPSOut
{
    float4 posW : SV_TARGET0;
    float4 normW    : SV_TARGET1;
    float4 albedo   : SV_TARGET2;
};

VSOut vsMain(VSIn vIn)
{
    return simpleVS(vIn);
}

GBufferPSOut psMain(VSOut vsOut, uint triangleIndex: SV_PrimitiveID) : SV_TARGET
{
    GBufferPSOut psOut;

    BasicMaterialData mbd = gScene.materials.getBasicMaterialData(vsOut.materialID);

    if (is_valid(gPosW))        psOut.posW = float4(vsOut.posW, 1.0);
    if (is_valid(gNormalW))     psOut.normW = float4(vsOut.normalW, 1.0);
    if (is_valid(gAlbedo))      psOut.albedo = float4(mbd.baseColor.r, mbd.baseColor.g, mbd.baseColor.b, mbd.baseColor.a);
    return psOut;
}