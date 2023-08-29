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

static const std::string kDefaultScene = "Sponza/NewSponza_Main_glTF_002.gltf";

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
    fs::path modelFullPath;
    if (!findFileInDataDirectories(kDefaultScene, modelFullPath))
    {
        throw RuntimeError("Can not find model file!");
    }

    loadScene(modelFullPath, getTargetFbo().get());
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
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void Sponza::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Falcor", {250, 200});
    renderGlobalUI(pGui);
    w.text("Hello from Sponza");
    if (w.button("Click Here"))
    {
        msgBox("Info", "Now why would you do that?");
    }
}

bool Sponza::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool Sponza::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void Sponza::onHotReload(HotReloadFlags reloaded)
{
    //
}

void Sponza::loadScene(const std::filesystem::path& path, const Fbo* pTargetFbo)
{
    mpScene = Scene::create(getDevice(), path);
    mpCamera = mpScene->getCamera();
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
