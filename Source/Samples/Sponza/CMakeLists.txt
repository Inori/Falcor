add_falcor_executable(Sponza)

target_sources(Sponza PRIVATE
    Sponza.cpp
    Sponza.h
    Sponza.slang
)

target_copy_shaders(Sponza Samples/Sponza)

target_source_group(Sponza "Samples")
