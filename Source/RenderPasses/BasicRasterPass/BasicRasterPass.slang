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

    float4 baseColor = ms.sampleTexture(bmd.texBaseColor, sampler, vsOut.texC, float4(0.0), lod);
    float4 normalMap = ms.sampleTexture(bmd.texNormalMap, sampler, vsOut.texC, float4(0.0), lod);
    float4 specular = ms.sampleTexture(bmd.texSpecular, sampler, vsOut.texC, float4(0.0), lod);

    float3 albedo = baseColor.rgb;
    float3 nomalT = normalMap.xyz;
    float3 normalW = vsOut.normalW;
    float opacity = baseColor.a;
    float roughness = specular.g;
    float metallic = specular.b;

    // LightData light = gScene.getLight(0); // The sky
    // float3 lightPos = light.posW;
    // float3 lightDir = normalize(lightPos - gScene.camera.getPosition());
    // float3 lightColor = light.intensity;
    LightData light = gScene.getLight(1); // The sun
    float3 lightDir = normalize(light.dirW);
    float3 lightColor = light.intensity;

    float3 ambient = albedo * 0.05;

    float3 diffuse = saturate(dot(normalW, lightDir)) * albedo;

    float3 viewDir = normalize(gScene.camera.getPosition() - vsOut.posW);
    float3 H = normalize(viewDir + lightDir);

    float cosTheta = pow(saturate(dot(normalW, H)), 20.0);
    float3 spec = lightColor * cosTheta;

    float3 color = ambient + diffuse + spec;

    return float4(color, 1.0);
}
