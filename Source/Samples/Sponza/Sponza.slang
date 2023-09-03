import Scene.Raster;

VSOut vsMain(VSIn vIn)
{
    VSOut vOut;
    const GeometryInstanceID instanceID = { vIn.instanceID };

    float4x4 worldMat = gScene.getWorldMatrix(instanceID);
    float4x4 viewProj = gScene.camera.getViewProj();

    float3 posW = mul(worldMat, float4(vIn.pos, 1.f)).xyz;
    vOut.posW = posW;
    vOut.posH = mul(gScene.camera.getViewProj(), float4(posW, 1.f));

    vOut.texC = vIn.texC;
    vOut.instanceID = instanceID;
    vOut.materialID = gScene.getMaterialID(instanceID);
    
    StaticVertexData svd = vIn.unpack();

    vOut.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), svd.normal);
    float4 tangent = svd.tangent;
    vOut.tangentW = float4(mul((float3x3)worldMat, tangent.xyz), tangent.w);
    vOut.prevPosH = float4(0.0);

    return vOut;
}

float4 psMain(VSOut vsOut, uint triangleIndex: SV_PrimitiveID) : SV_TARGET
{
    MaterialSystem ms = gScene.materials;
    let lod = ImplicitLodTextureSampler();
    SamplerState sampler = ms.getDefaultTextureSampler(vsOut.materialID);
    BasicMaterialData bmd = ms.getBasicMaterialData(vsOut.materialID);

    float4 diffuse = ms.sampleTexture(bmd.texBaseColor, sampler, vsOut.texC, float4(0.0), lod);
    return float4(diffuse.xyz, 1.0);
}