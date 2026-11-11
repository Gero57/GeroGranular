#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>

class GeroGrainAudioProcessor final : public juce::AudioProcessor
{
public:
    GeroGrainAudioProcessor();
    ~GeroGrainAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "GeroGrain"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    bool loadSample (const juce::File& file);
    juce::String getSampleName() const { return sampleName; }

private:
    struct Grain
    {
        bool active = false;
        double position = 0.0;
        double increment = 1.0;
        int age = 0;
        int length = 1;
        float gain = 0.0f;
        float pan = 0.0f;
        int midiNote = 60;
        bool fromFeedback = false;
    };

    juce::AudioFormatManager formats;
    juce::AudioBuffer<float> source;
    juce::AudioBuffer<float> feedbackBuffer;
    juce::AudioBuffer<float> effectBuffer;
    int effectWritePosition = 0;
    double chorusPhase = 0.0;
    int feedbackWritePosition = 0;
    int feedbackSamplesWritten = 0;
    juce::String sampleName { "No sample loaded" };
    juce::CriticalSection sampleLock;
    std::array<Grain, 256> grains {};
    std::array<bool, 128> heldNotes {};
    std::array<float, 128> noteVelocities {};
    std::array<int, 128> noteSpawnCounters {};
    std::array<float, 128> noteEnvelopes {};
    std::array<bool, 128> noteReleasing {};
    std::array<bool, 128> noteDecaying {};
    std::array<float, 2> filterLow {};
    std::array<float, 2> filterBand {};
    juce::Reverb reverb;
    double currentSampleRate = 44100.0;
    double sourceSampleRate = 44100.0;
    double lfoPhase = 0.0;
    float lfoValue = 0.0f;
    float lfoRandomValue = 0.0f;
    juce::Random random;

    void spawnGrain (int midiNote, float velocity);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeroGrainAudioProcessor)
};
