/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "LightingPass.h"
#include "RenderGraph/RenderPassHelpers.h"


extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, LightingPass>();
}

namespace
{
// clang-format off
    const char kDst[] = "dst";

    const std::string kLightingPassProgramFile = "RenderPasses/LightingPass/LightingPass.slang";
   
    const ChannelList kGBufferChannels =
    {
        {"posW",    "gPosW",           "Position in world space",  true /* optional */, ResourceFormat::RGBA32Float},
        {"normalW", "gNormalW",        "Normal in world space",    true /* optional */, ResourceFormat::RGBA32Float},
        {"texC",    "gTexC",           "Texture coordinate",       true /* optional */, ResourceFormat::RG32Float},
        {"mtlData", "gMaterialData",   "Material information",     true /* optional */, ResourceFormat::RGBA32Uint},
    };
    
    const std::string kDepthName = "depth";

// clang-format on
} // namespace

LightingPass::LightingPass(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    mpFbo = Fbo::create(pDevice);
}

Properties LightingPass::getProperties() const
{
    return {};
}

RenderPassReflection LightingPass::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    addRenderPassInputs(reflector, kGBufferChannels);
    reflector.addOutput(kDst, "Output texture").bindFlags(ResourceBindFlags::RenderTarget).format(ResourceFormat::RGBA32Float);
    return reflector;
}

void LightingPass::compile(RenderContext* pRenderContext, const CompileData& compileData) {}

void LightingPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (!mpScene)
        return;

    // renderData holds the requested resources
    auto pDst = renderData.getTexture(kDst);
    FALCOR_ASSERT(pDst);

    auto var = mpLightingPass->getRootVar();

    mpScene->setRasterizeShaderData(pRenderContext, var);

    for (const auto& channel : kGBufferChannels)
    {
        var[channel.texname] = renderData.getTexture(channel.name);
    }

    mpFbo->attachColorTarget(pDst, 0);
    mpLightingPass->execute(pRenderContext, mpFbo);
}

void LightingPass::renderUI(Gui::Widgets& widget)
{
}

void LightingPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;

    if (!mpScene)
        return;

    auto shaderModules = mpScene->getShaderModules();
    auto typeConformances = mpScene->getTypeConformances();
    auto defines = mpScene->getSceneDefines();

    Program::Desc progDesc;
    progDesc.addShaderModules(shaderModules);
    progDesc.addTypeConformances(typeConformances);
    progDesc.addShaderLibrary(kLightingPassProgramFile).psEntry("main");

    mpLightingPass = FullScreenPass::create(pRenderContext->getDevice(), progDesc, defines);
}

bool LightingPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

bool LightingPass::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}
