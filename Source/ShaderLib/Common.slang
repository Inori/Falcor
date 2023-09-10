__exported import Scene.Raster;

VSOut simpleVS(VSIn vIn)
{
    VSOut vOut;
    const GeometryInstanceID instanceID = { vIn.instanceID };

    float4x4 worldMat = gScene.getWorldMatrix(instanceID);
    float4x4 viewProjMat = gScene.camera.getViewProj();

    StaticVertexData svd = vIn.unpack();

    vOut.posW = mul(worldMat, float4(vIn.pos, 1.0)).xyz;
    vOut.posH = mul(viewProjMat, float4(vOut.posW, 1.0));
    vOut.normalW = mul(gScene.getInverseTransposeWorldMatrix(instanceID), svd.normal);
    vOut.texC = vIn.texC;
    vOut.materialID = gScene.getMaterialID(instanceID);

    return vOut;
}