cmake_minimum_required(VERSION 3.22)

project(XSide VERSION 4.6.0)

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.7
)

FetchContent_MakeAvailable(JUCE)

juce_add_plugin(XSide
    COMPANY_NAME "X-Side"
    BUNDLE_ID "com.xside.dsp1000p"

    PLUGIN_MANUFACTURER_CODE XSde
    PLUGIN_CODE Xdsp

    FORMATS
        VST3
        Standalone

    PRODUCT_NAME "X-Side"
    PLUGIN_NAME "X-Side"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(XSide)

juce_add_binary_data(XSideData
    SOURCES
        Resources/factory_presets.json
)

target_sources(XSide
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_features(XSide PRIVATE cxx_std_17)

target_compile_definitions(XSide
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(XSide
    PRIVATE
        XSideData
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_data_structures
)
