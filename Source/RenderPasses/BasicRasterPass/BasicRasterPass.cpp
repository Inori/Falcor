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
#include "BasicRasterPass.h"

namespace
{
    const char kDst[] = "dst";
    const char kDepth[] = "depth";

    void regBasicRasterPass(pybind11::module& m)
    {
        pybind11::class_<BasicRasterPass, RenderPass, ref<BasicRasterPass>> pass(m, "BasicRasterPass");
    }
} // namespace

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, BasicRasterPass>();
    ScriptBindings::registerBinding(regBasicRasterPass);
}

ref<BasicRasterPass> BasicRasterPass::create(ref<Device> pDevice, const Properties& props)
{
    return make_ref<BasicRasterPass>(pDevice, props);
}

BasicRasterPass::BasicRasterPass(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    mpFbo = Fbo::create(mpDevice);
}

Properties BasicRasterPass::getProperties() const
{
    return {};
}

RenderPassReflection BasicRasterPass::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addOutput(kDst, "The final output.").bindFlags(Resource::BindFlags::RenderTarget);
    // we have to do this because we have no way to get final depth buffer
    reflector.addInternal(kDepth, "Internal depth buffer.").format(ResourceFormat::D32Float).bindFlags(Resource::BindFlags::DepthStencil);

    return reflector;
}

void BasicRasterPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
}

void BasicRasterPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");
    if (!mpScene) return;

    const auto& pDepth = renderData.getTexture(kDepth);
    const auto& pDstTex = renderData.getTexture(kDst);

    mpFbo->attachDepthStencilTarget(pDepth);
    mpFbo->attachColorTarget(pDstTex, 0);

    pRenderContext->clearFbo(mpFbo.get(), float4(0), 1.f, 0, FboAttachmentType::All);

    mpRasterPass->getState()->setFbo(mpFbo);
    mpScene->rasterize(pRenderContext, mpRasterPass->getState().get(), mpRasterPass->getVars().get());
}

void BasicRasterPass::renderUI(Gui::Widgets& widget)
{
}

void BasicRasterPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
    setupScene(pRenderContext);
}

bool BasicRasterPass::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

bool BasicRasterPass::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

void BasicRasterPass::setupScene(RenderContext* pRenderContext)
{
    if (!mpScene) return;

    // setup camera
    mpCamera = mpScene->getCamera();
    float radius = mpScene->getSceneBounds().radius();
    mpScene->setCameraSpeed(radius * 0.5f);
    float nearZ = std::max(0.1f, radius / 750.0f);
    float farZ = radius * 10;
    mpCamera->setDepthRange(nearZ, farZ);

    // setup raster pass
    auto shaderModules = mpScene->getShaderModules();
    auto typeConformances = mpScene->getTypeConformances();
    auto defines = mpScene->getSceneDefines();

    Program::Desc rasterProgDesc;
    rasterProgDesc.addShaderModules(shaderModules);
    rasterProgDesc.addTypeConformances(typeConformances);
    rasterProgDesc.addShaderLibrary("RenderPasses/BasicRasterPass/BasicRasterPass.slang").vsEntry("vsMain").psEntry("psMain");

    mpRasterPass = RasterPass::create(pRenderContext->getDevice(), rasterProgDesc, defines);
}
