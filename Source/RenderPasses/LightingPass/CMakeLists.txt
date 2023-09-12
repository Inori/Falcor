add_plugin(LightingPass)

target_sources(LightingPass PRIVATE
    LightingPass.cpp
    LightingPass.h
    LightingPass.slang
)

target_copy_shaders(LightingPass RenderPasses/LightingPass)

target_source_group(LightingPass "RenderPasses")
