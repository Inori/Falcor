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
#include "GBufferPass.h"
#include "RenderGraph/RenderPassHelpers.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, GBufferPass>();
}

namespace
{
// clang-format off

    const std::string kDepthPassProgramFile = "RenderPasses/GBufferPass/DepthOnlyPass.slang";
    const std::string kGBufferPassProgramFile = "RenderPasses/GBufferPass/GBufferPass.slang";
   
    const ChannelList kGBufferChannels =
    {
        {"posW",    "gPosW",    "position in world space",  true /* optional */, ResourceFormat::RGBA32Float},
        {"normalW", "gNormalW", "normal in world space",    true /* optional */, ResourceFormat::RGBA32Float},
        {"albedo",  "gAlbedo",  "albedo",                   true /* optional */, ResourceFormat::RGBA32Float},
    };

    const std::string kDepthName = "depth";

// clang-format on
}


GBufferPass::GBufferPass(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    mpFbo = Fbo::create(pDevice);

    // Depth pipeline
    {
        mDepthPipeline.pState = GraphicsState::create(pDevice);
        // Disable color write for depth only pass.
        BlendState::Desc bsDesc;
        bsDesc.setRenderTargetWriteMask(0, 0, 0, 0, 0);
        ref<BlendState> bsState = BlendState::create(bsDesc);
        mDepthPipeline.pState->setBlendState(bsState);
    }

    // GBuffer pipeline
    {
        mGBufferPipeline.pState = GraphicsState::create(pDevice);
        // Use equal depth test and disable depth write
        DepthStencilState::Desc dsDesc;
        dsDesc.setDepthFunc(DepthStencilState::Func::Equal).setDepthWriteMask(false);
        ref<DepthStencilState> dsState = DepthStencilState::create(dsDesc);
        mGBufferPipeline.pState->setDepthStencilState(dsState);
    }
}

Properties GBufferPass::getProperties() const
{
    return {};
}

RenderPassReflection GBufferPass::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;

    // Depth buffer
    reflector.addOutput(kDepthName, "depth buffer").bindFlags(ResourceBindFlags::DepthStencil).format(ResourceFormat::D32Float);

    // GBuffer
    addRenderPassOutputs(reflector, kGBufferChannels, ResourceBindFlags::RenderTarget);

    return reflector;
}

void GBufferPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
}

void GBufferPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");

    if (!mpScene)
        return;

    detachFboTargets();

    // Render depth only pass
    {
        auto pDepthTex = renderData.getTexture(kDepthName);
        FALCOR_ASSERT(pDepthTex);
        mpFbo->attachDepthStencilTarget(pDepthTex);

        pRenderContext->clearFbo(mpFbo.get(), {}, 1.0f, 0, FboAttachmentType::Depth | FboAttachmentType::Stencil);

        // Create program vars.
        if (!mDepthPipeline.pVars)
            mDepthPipeline.pVars = GraphicsVars::create(mpDevice, mDepthPipeline.pProgram.get());

        mDepthPipeline.pState->setFbo(mpFbo);
        mpScene->rasterize(pRenderContext, mDepthPipeline.pState.get(), mDepthPipeline.pVars.get());
    }

    // Render gbuffer pass
    {
        auto defines = getValidResourceDefines(kGBufferChannels, renderData);
        mGBufferPipeline.pProgram->addDefines(defines);

        for (size_t rtIdx = 0; rtIdx != kGBufferChannels.size(); ++rtIdx)
        {
            auto pTarget = renderData.getTexture(kGBufferChannels[rtIdx].name);
            if (!pTarget)
            {
                continue;
            }
            mpFbo->attachColorTarget(pTarget, rtIdx);
        }

        pRenderContext->clearFbo(mpFbo.get(), {}, 1.0f, 0, FboAttachmentType::Color);

        // Create program vars.
        if (!mGBufferPipeline.pVars)
            mGBufferPipeline.pVars = GraphicsVars::create(mpDevice, mGBufferPipeline.pProgram.get());

        mGBufferPipeline.pState->setFbo(mpFbo);
        mpScene->rasterize(pRenderContext, mGBufferPipeline.pState.get(), mGBufferPipeline.pVars.get());
    }
}

void GBufferPass::renderUI(Gui::Widgets& widget)
{
}

void GBufferPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    if (!pScene)
        return;

    mpScene = pScene;

    auto shaderModules = mpScene->getShaderModules();
    auto typeConformances = mpScene->getTypeConformances();
    auto defines = mpScene->getSceneDefines();

    // Depth
    {
        Program::Desc progDesc;
        progDesc.addShaderModules(shaderModules);
        progDesc.addTypeConformances(typeConformances);
        progDesc.addShaderLibrary(kDepthPassProgramFile).vsEntry("vsMain").psEntry("psMain");

        mDepthPipeline.pProgram = GraphicsProgram::create(pRenderContext->getDevice(), progDesc, defines);
        mDepthPipeline.pState->setProgram(mDepthPipeline.pProgram);
    }

    // GBuffer
    {
        Program::Desc progDesc;
        progDesc.addShaderModules(shaderModules);
        progDesc.addTypeConformances(typeConformances);
        progDesc.addShaderLibrary(kGBufferPassProgramFile).vsEntry("vsMain").psEntry("psMain");

        mGBufferPipeline.pProgram = GraphicsProgram::create(pRenderContext->getDevice(), progDesc, defines);
        mGBufferPipeline.pState->setProgram(mGBufferPipeline.pProgram);
    }

}

bool GBufferPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

bool GBufferPass::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

void GBufferPass::detachFboTargets()
{
    FALCOR_ASSERT(mpFbo);

    mpFbo->attachDepthStencilTarget(nullptr);

    uint32_t colorCount = mpFbo->getMaxColorTargetCount();
    for (uint32_t idx = 0; idx != colorCount; ++idx)
    {
        mpFbo->attachColorTarget(nullptr, idx);
    }
}
