add_plugin(BasicRasterPass)

target_sources(BasicRasterPass PRIVATE
    BasicRasterPass.cpp
    BasicRasterPass.h
    BasicRasterPass.slang
)

target_source_group(BasicRasterPass "RenderPasses")
