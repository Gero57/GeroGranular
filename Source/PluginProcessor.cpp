#include "PluginProcessor.h"
#include "PluginEditor.h"

GeroGrainAudioProcessor::GeroGrainAudioProcessor()
 : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
   parameters (*this, nullptr, "PARAMETERS", {
      std::make_unique<juce::AudioParameterFloat>("size","Grain Size",juce::NormalisableRange<float>(0.02f,0.50f,0.001f,0.45f),0.12f),
      std::make_unique<juce::AudioParameterFloat>("density","Density",juce::NormalisableRange<float>(2.0f,80.0f,0.1f),24.0f),
      std::make_unique<juce::AudioParameterFloat>("position","Position",0.0f,1.0f,0.5f),
      std::make_unique<juce::AudioParameterFloat>("random","Random",0.0f,1.0f,0.15f),
      std::make_unique<juce::AudioParameterFloat>("pitch","Pitch",-24.0f,24.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("spread","Spread",0.0f,1.0f,0.65f),
      std::make_unique<juce::AudioParameterFloat>("attack","Attack",0.001f,2.0f,0.02f),
      std::make_unique<juce::AudioParameterFloat>("release","Release",0.01f,5.0f,0.25f),
      std::make_unique<juce::AudioParameterFloat>("cutoff","Cutoff",40.0f,18000.0f,14000.0f),
      std::make_unique<juce::AudioParameterFloat>("resonance","Resonance",0.1f,1.0f,0.25f),
      std::make_unique<juce::AudioParameterFloat>("reverb","Reverb",0.0f,1.0f,0.15f),
      std::make_unique<juce::AudioParameterFloat>("jitter","Jitter",0.0f,1.0f,0.1f),
      std::make_unique<juce::AudioParameterFloat>("smooth","Smooth",0.0f,1.0f,0.5f),
      std::make_unique<juce::AudioParameterFloat>("stretch","Stretch",0.25f,4.0f,1.0f),
      std::make_unique<juce::AudioParameterFloat>("stereo","Stereo",0.0f,1.0f,1.0f),
      std::make_unique<juce::AudioParameterFloat>("feedback","Feedback",0.0f,0.95f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("fine","Fine",-100.0f,100.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("lfoamt","LFO Amount",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("sustain","Sustain",0.0f,1.0f,0.8f),
      std::make_unique<juce::AudioParameterFloat>("lforate","LFO Rate",0.05f,20.0f,1.0f),
      std::make_unique<juce::AudioParameterFloat>("lfodepth","LFO Depth",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("lfoshape","LFO Shape",0.0f,4.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("modlfo1","Mod LFO 1",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("modlfo2","Mod LFO 2",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("modenv","Mod Envelope",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("drive","Drive",0.0f,24.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("chorus","Chorus",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("delay","Delay",0.0f,1.0f,0.0f),
      std::make_unique<juce::AudioParameterFloat>("effectmix","Effect Mix",0.0f,1.0f,0.25f),
      std::make_unique<juce::AudioParameterFloat>("decay","Decay",0.001f,3.0f,0.25f)
   })
{
    formats.registerBasicFormats();
}

bool GeroGrainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void GeroGrainAudioProcessor::prepareToPlay (double sr, int)
{
    currentSampleRate = sr;
    noteSpawnCounters.fill (0);
    for (auto& g : grains) g.active = false;
}

bool GeroGrainAudioProcessor::loadSample (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor(file));
    if (reader == nullptr) return false;
    juce::AudioBuffer<float> loaded ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&loaded, 0, (int) reader->lengthInSamples, 0, true, true);
    const juce::ScopedLock lock (sampleLock);
    source = std::move (loaded);
    sourceSampleRate = reader->sampleRate;
    sampleName = file.getFileName();
    for (auto& g : grains) g.active = false;
    return true;
}

void GeroGrainAudioProcessor::spawnGrain (int note, float velocity)
{
    if (source.getNumSamples() < 2) return;
    for (auto& g : grains)
    {
        if (g.active) continue;
        const auto size = parameters.getRawParameterValue("size")->load();
        const auto pos = parameters.getRawParameterValue("position")->load();
        const auto rnd = parameters.getRawParameterValue("random")->load();
        const auto pitch = parameters.getRawParameterValue("pitch")->load();
        const auto spread = parameters.getRawParameterValue("spread")->load();
        const double centre = juce::jlimit(0.0, 1.0, (double) pos + random.nextFloat() * rnd - rnd * 0.5);
        g.position = centre * (source.getNumSamples() - 1);
        g.length = juce::jmax(64, (int)(size * currentSampleRate));
        g.age = 0;
        const double pitchRatio = std::pow(2.0, (note - 60 + pitch) / 12.0);
        const double sampleRateRatio = sourceSampleRate / juce::jmax (1.0, currentSampleRate);
        g.increment = pitchRatio * sampleRateRatio;
        g.gain = velocity * 0.22f;
        g.pan = (random.nextFloat() * 2.0f - 1.0f) * spread;
        g.active = true;
        break;
    }
}

void GeroGrainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const juce::ScopedTryLock lock (sampleLock);
    if (!lock.isLocked() || source.getNumSamples() < 2) return;

    const float density = parameters.getRawParameterValue("density")->load();
    const int interval = juce::jmax (1, (int) (currentSampleRate / juce::jmax (1.0f, density)));

    const int total = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    auto midiEvent = midi.begin();
    const auto midiEnd = midi.end();

    for (int i = 0; i < total; ++i)
    {
        // Apply MIDI events at their exact sample offset inside this block.
        while (midiEvent != midiEnd && (*midiEvent).samplePosition <= i)
        {
            const auto msg = (*midiEvent).getMessage();
            const int note = msg.getNoteNumber();

            if (note >= 0 && note < 128)
            {
                const auto noteIndex = (size_t) note;

                if (msg.isNoteOn())
                {
                    heldNotes[noteIndex] = true;
                    noteVelocities[noteIndex] = msg.getFloatVelocity();
                    noteSpawnCounters[noteIndex] = 0;
                    spawnGrain (note, noteVelocities[noteIndex]);
                }
                else if (msg.isNoteOff())
                {
                    heldNotes[noteIndex] = false;
                    noteSpawnCounters[noteIndex] = 0;
                }
            }

            ++midiEvent;
        }

        // Every held note has its own independent grain-generation clock.
        for (int n = 0; n < 128; ++n)
        {
            const auto noteIndex = (size_t) n;
            if (! heldNotes[noteIndex])
                continue;

            if (++noteSpawnCounters[noteIndex] >= interval)
            {
                noteSpawnCounters[noteIndex] = 0;
                spawnGrain (n, noteVelocities[noteIndex]);
            }
        }
        for (auto& g : grains)
        {
            if (!g.active) continue;
            if (g.age >= g.length || g.position >= source.getNumSamples()-1) { g.active = false; continue; }
            const float phase = (float)g.age / (float)g.length;
            const float window = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phase);
            const int idx = (int)g.position;
            const float frac = (float)(g.position - idx);
            const int nextIdx = juce::jmin (idx + 1, source.getNumSamples() - 1);
            const int sourceChannels = source.getNumChannels();
            const float leftA = source.getSample (0, idx);
            const float leftB = source.getSample (0, nextIdx);
            const float leftSample = leftA + (leftB - leftA) * frac;
            const float rightA = source.getSample (sourceChannels > 1 ? 1 : 0, idx);
            const float rightB = source.getSample (sourceChannels > 1 ? 1 : 0, nextIdx);
            const float rightSample = rightA + (rightB - rightA) * frac;
            const float panAngle = (g.pan + 1.0f) * 0.7853981633974483f;
            const float leftGain = std::cos (panAngle) * 1.41421356f;
            const float rightGain = std::sin (panAngle) * 1.41421356f;
            const float envelopeGain = window * g.gain;
            if (channels == 1)
                buffer.addSample (0, i, 0.5f * (leftSample * leftGain + rightSample * rightGain) * envelopeGain);
            else
            {
                buffer.addSample (0, i, leftSample * leftGain * envelopeGain);
                buffer.addSample (1, i, rightSample * rightGain * envelopeGain);
            }
            g.position += g.increment;
            ++g.age;
        }
    }
}

juce::AudioProcessorEditor* GeroGrainAudioProcessor::createEditor() { return new GeroGrainAudioProcessorEditor(*this); }
void GeroGrainAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream stream(dest, true);
    parameters.state.writeToStream(stream);
    stream.writeString(sampleName);
}
void GeroGrainAudioProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree:: readFromData(data, (size_t)size);
    if (tree.isValid()) parameters.replaceState(tree);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GeroGrainAudioProcessor(); }
