#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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
    juce::Image background;
    juce::TextButton loadButton { "LOAD SOURCE" };
    juce::Label sampleLabel;
    std::array<std::unique_ptr<NeonKnob>, 11> knobs;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 11> attachments;
    const std::array<juce::String,11> ids { "size","density","spread","random","position","pitch","attack","release","cutoff","resonance","reverb" };
    void chooseSample();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeroGrainAudioProcessorEditor)
};
