cmake_minimum_required(VERSION 3.22)
project(GeroGrain VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GeroGrain
  COMPANY_NAME "GeroAudio"
  BUNDLE_ID "com.geroaudio.gerograin"
  PLUGIN_MANUFACTURER_CODE Gero
  PLUGIN_CODE GGrn
  FORMATS VST3 AU
  PRODUCT_NAME "GeroGrain"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(GeroGrain)

juce_add_binary_data(GeroGrainAssets SOURCES BaseGrain.svg)

target_sources(GeroGrain PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)
target_include_directories(GeroGrain PRIVATE Source)
target_link_libraries(GeroGrain PRIVATE
  GeroGrainAssets
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_gui_extra
  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags)
