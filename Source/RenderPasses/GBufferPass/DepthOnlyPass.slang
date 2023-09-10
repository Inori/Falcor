import ShaderLib.Common;

VSOut vsMain(VSIn vIn)
{
    return simpleVS(vIn);
}

float4 psMain(VSOut vsOut, uint triangleIndex: SV_PrimitiveID) : SV_TARGET
{
    return float4(0.0);
}