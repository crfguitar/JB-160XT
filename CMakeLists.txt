cmake_minimum_required(VERSION 3.22)

project(JB160XT VERSION 0.1.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(JUCE_DIR_PATH "C:/JUCE" CACHE PATH "Path to the JUCE checkout")
option(JB160_BUILD_PLUGIN "Build the VST3/Standalone plugin (needs JUCE)" ON)

#==============================================================================
# DSP checks: plain C++, no JUCE.
add_executable(jb160_dsp_tests Tests/dsp_tests.cpp)
if(MSVC)
    target_compile_options(jb160_dsp_tests PRIVATE /W4 /fp:precise)
endif()

if(NOT JB160_BUILD_PLUGIN)
    return()
endif()

#==============================================================================
add_subdirectory("${JUCE_DIR_PATH}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

juce_add_plugin(JB160XT
    COMPANY_NAME "JB"
    PLUGIN_MANUFACTURER_CODE Jbfx
    PLUGIN_CODE J16x
    FORMATS VST3 Standalone
    PRODUCT_NAME "JB 160XT"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Dynamics
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

juce_add_binary_data(JB160XTAssets
    NAMESPACE Assets
    SOURCES Assets/fonts/Michroma-Regular.ttf)

set(JB160_PLUGIN_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/Faceplate.cpp)

target_sources(JB160XT PRIVATE ${JB160_PLUGIN_SOURCES})

target_compile_definitions(JB160XT PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(JB160XT
    PRIVATE
        JB160XTAssets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

#==============================================================================
# Renders the editor to a PNG so the faceplate can be compared with the photos.
juce_add_console_app(jb160_render_panel PRODUCT_NAME "jb160_render_panel")

target_sources(jb160_render_panel PRIVATE
    Tools/render_panel.cpp
    ${JB160_PLUGIN_SOURCES})

target_compile_definitions(jb160_render_panel PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JucePlugin_Name="JB 160XT"
    JucePlugin_Build_Standalone=1
    JB160_RENDER_TOOL=1)

target_link_libraries(jb160_render_panel
    PRIVATE
        JB160XTAssets
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags)
