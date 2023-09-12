add_plugin(GBufferPass)

target_sources(GBufferPass PRIVATE
    GBufferPass.cpp
    GBufferPass.h
    DepthOnlyPass.slang
    GBufferPass.slang
)

target_copy_shaders(GBufferPass RenderPasses/GBufferPass)

target_source_group(GBufferPass "RenderPasses")
