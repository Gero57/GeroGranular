#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class NeonKnob final : public juce::Slider
{
public:
    NeonKnob();
    void paint (juce::Graphics&) override;
};

class GeroGrainAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit GeroGrainAudioProcessorEditor (GeroGrainAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    GeroGrainAudioProcessor& processor;
    juce::TextButton loadButton { "LOAD" };
    juce::Label sampleLabel;
    static constexpr size_t knobCount = 30;
    std::array<std::unique_ptr<NeonKnob>, knobCount> knobs;
    std::array<std::unique_ptr<juce::Label>, knobCount> knobLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, knobCount> attachments;
    const std::array<juce::String, knobCount> ids {
        "size","density","spread","random","jitter","smooth",
        "position","stretch","stereo","feedback",
        "pitch","fine","lfoamt",
        "attack","decay","sustain","release",
        "lforate","lfodepth","lfoshape",
        "modlfo1","modlfo2","modenv",
        "cutoff","resonance","drive",
        "chorus","delay","reverb","effectmix"
    };
    const std::array<juce::String, knobCount> names {
        "SIZE","DENSITY","SPREAD","RANDOM","JITTER","SMOOTH",
        "POSITION","STRETCH","STEREO","FEEDBACK",
        "PITCH","FINE","LFO AMT",
        "ATTACK","DECAY","SUSTAIN","RELEASE",
        "RATE","DEPTH","SHAPE",
        "LFO 1","LFO 2","ENV",
        "CUTOFF","RESONANCE","DRIVE",
        "CHORUS","DELAY","REVERB","MIX"
    };
    void chooseSample();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeroGrainAudioProcessorEditor)
};
