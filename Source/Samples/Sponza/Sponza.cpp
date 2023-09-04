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
#include "Sponza.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

namespace fs = std::filesystem;

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

static const std::string kSceneSponzaMain = "Sponza/NewSponza_Main_glTF_002.gltf";
static const std::string kSceneSponzaCurtains = "Sponza/NewSponza_Curtains_glTF.gltf";
static const std::string kSceneSettingFile = "sponza_settings.json";

Sponza::Sponza(const SampleAppConfig& config) : SampleApp(config)
{
    //
}

Sponza::~Sponza()
{
    //
}

void Sponza::onLoad(RenderContext* pRenderContext)
{
    //
    createScene();
    setupScene();
}

void Sponza::onShutdown()
{
    //
}

void Sponza::onResize(uint32_t width, uint32_t height)
{
    //
}

void Sponza::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    FALCOR_ASSERT(mpScene);

    const float4 clearColor(0.0f, 0.64f, 0.95f, 1.0f);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    mpScene->update(pRenderContext, getGlobalClock().getTime());

    renderRaster(pRenderContext, pTargetFbo);
}

void Sponza::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Falcor", {250, 200});
    renderGlobalUI(pGui);
    w.text("Hello Sponza");
    if (w.button("Click Here"))
    {
        msgBox("Info", "Now why would you do that?");
    }
}

bool Sponza::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return mpScene && mpScene->onKeyEvent(keyEvent);
}

bool Sponza::onMouseEvent(const MouseEvent& mouseEvent)
{
    return mpScene && mpScene->onMouseEvent(mouseEvent);
}

void Sponza::onHotReload(HotReloadFlags reloaded)
{
    //
}

void Sponza::createScene()
{
    Settings settings;
    settings.addOptions(getRuntimeDirectory() / kSceneSettingFile);

    std::vector<std::filesystem::path> sceneFiles = {kSceneSponzaMain, kSceneSponzaCurtains};
    mpScene = Scene::create(getDevice(), sceneFiles, settings);
}

void Sponza::setupScene()
{
    Fbo* pTargetFbo = getTargetFbo().get();

    // setup camera
    mpCamera = mpScene->getCamera();
    float radius = mpScene->getSceneBounds().radius();
    mpScene->setCameraSpeed(radius * 0.5f);
    float nearZ = std::max(0.1f, radius / 750.0f);
    float farZ = radius * 10;
    mpCamera->setDepthRange(nearZ, farZ);
    mpCamera->setAspectRatio((float)pTargetFbo->getWidth() / (float)pTargetFbo->getHeight());

    // setup raster pass
    auto shaderModules = mpScene->getShaderModules();
    auto typeConformances = mpScene->getTypeConformances();
    auto defines = mpScene->getSceneDefines();

    Program::Desc rasterProgDesc;
    rasterProgDesc.addShaderModules(shaderModules);
    rasterProgDesc.addTypeConformances(typeConformances);
    rasterProgDesc.addShaderLibrary("Samples/Sponza/Sponza.slang").vsEntry("vsMain").psEntry("psMain");

    mpRasterPass = RasterPass::create(getDevice(), rasterProgDesc, defines);
}

void Sponza::renderRaster(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    FALCOR_PROFILE(pRenderContext, "renderRaster");

    mpRasterPass->getState()->setFbo(pTargetFbo);
    mpScene->rasterize(pRenderContext, mpRasterPass->getState().get(), mpRasterPass->getVars().get());
}

int main(int argc, char** argv)
{
    SampleAppConfig config;
    // config.deviceDesc.type = Device::Type::Vulkan;
    config.windowDesc.title = "Excellent Sponza";
    config.windowDesc.resizableWindow = true;

    Sponza project(config);
    return project.run();
}
