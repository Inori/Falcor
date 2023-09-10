add_custom_target(ShaderLib)

target_sources(ShaderLib PRIVATE
    Common.slang
)

target_copy_shaders(ShaderLib ShaderLib)

target_source_group(ShaderLib "/")
