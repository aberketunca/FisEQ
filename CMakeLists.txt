cmake_minimum_required(VERSION 3.22)

project(FisEQ VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(FisEQ
    COMPANY_NAME "terkebunca"
    BUNDLE_ID "com.terkebunca.fiseq"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    PLUGIN_MANUFACTURER_CODE BrkT
    PLUGIN_CODE FisE

    FORMATS Standalone VST3
    VST3_AUTO_MANIFEST FALSE

    PRODUCT_NAME "FisEQ"
)

target_sources(FisEQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    
    Source/DSP/EQBand.cpp
    Source/DSP/EQBand.h
)

target_compile_definitions(FisEQ PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0

    JUCE_VST3_CAN_REPLACE_VST2=0

)

target_link_libraries(FisEQ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp

    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)