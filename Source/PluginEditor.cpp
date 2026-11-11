#include "PluginEditor.h"
#include "BinaryData.h"

NeonKnob::NeonKnob()
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                        juce::MathConstants<float>::pi * 2.8f, true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void NeonKnob::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (5.0f);
    const auto centre = b.getCentre();
    const float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.43f;

    // Corpo opaco: copre il quadrante statico stampato nello sfondo.
    g.setColour (juce::Colour (0xff07131a));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (juce::Colour (0xff00dfff));
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.2f);
    g.setColour (juce::Colour (0xff294b55));
    g.drawEllipse (centre.x - radius + 4.0f, centre.y - radius + 4.0f,
                   radius * 2.0f - 8.0f, radius * 2.0f - 8.0f, 1.2f);

    const float proportion = (float) ((getValue() - getMinimum()) /
                                      juce::jmax (0.000001, getMaximum() - getMinimum()));
    const float angle = juce::MathConstants<float>::pi * 1.2f
                      + proportion * juce::MathConstants<float>::pi * 1.6f;
    const float pointerRadius = radius * 0.70f;
    juce::Path pointer;
    pointer.startNewSubPath (centre.x, centre.y);
    pointer.lineTo (centre.x + std::cos (angle) * pointerRadius,
                    centre.y + std::sin (angle) * pointerRadius);
    g.setColour (juce::Colour (0xfff2ffff));
    g.strokePath (pointer, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xff00dfff));
    g.fillEllipse (centre.x - 3.0f, centre.y - 3.0f, 6.0f, 6.0f);
}

GeroGrainAudioProcessorEditor::GeroGrainAudioProcessorEditor (GeroGrainAudioProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    background = juce::ImageCache::getFromMemory(BinaryData::BaseGranulare_png, BinaryData::BaseGranulare_pngSize);
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] { chooseSample(); };
    addAndMakeVisible(sampleLabel);
    sampleLabel.setText(processor.getSampleName(), juce::dontSendNotification);
    sampleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd8fbff));
    sampleLabel.setJustificationType(juce::Justification::centredLeft);
    sampleLabel.setFont (juce::Font (13.0f));

    for (size_t i=0; i<knobs.size(); ++i)
    {
        knobs[i] = std::make_unique<NeonKnob>();
        addAndMakeVisible(*knobs[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (processor.parameters, ids[i], *knobs[i]);
    }
    setSize(1200, 800);
}

void GeroGrainAudioProcessorEditor::chooseSample()
{
    auto chooser = std::make_shared<juce::FileChooser>("Load audio source", juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.ogg");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc)
        {
            if (fc.getResult().existsAsFile() && processor.loadSample(fc.getResult()))
                sampleLabel.setText(processor.getSampleName(), juce::dontSendNotification);
        });
}

void GeroGrainAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (background.isValid()) g.drawImage (background, getLocalBounds().toFloat());
    else { g.fillAll(juce::Colour(0xff071018)); g.setColour(juce::Colours::white); g.drawText("GeroGrain", getLocalBounds(), juce::Justification::centred); }
}

void GeroGrainAudioProcessorEditor::resized()
{
    const float sx = getWidth() / 1300.0f, sy = getHeight() / 867.0f;
    auto place = [sx,sy](juce::Component& c, float x, float y, float w, float h)
    {
        c.setBounds ((int)(x*sx), (int)(y*sy), (int)(w*sx), (int)(h*sy));
    };

    // Il pulsante e il nome del file seguono gli elementi già disegnati nel pannello.
    place (loadButton, 344, 147, 42, 42);
    loadButton.setButtonText ("");
    loadButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff08232c));
    loadButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    place (sampleLabel, 104, 147, 235, 42);

    // Centri delle manopole stampate sul pannello (coordinate immagine 1300 x 867).
    const std::array<juce::Point<float>, 11> centres {{
        {951, 189},   // Size
        {1075, 189},  // Density
        {1200, 189},  // Spread
        {951, 306},   // Random
        {1075, 306},  // Position, per il prototipo
        {99, 497},    // Pitch
        {601, 520},   // Attack
        {858, 520},   // Release
        {1033, 516},  // Cutoff
        {1180, 516},  // Resonance
        {1216, 670}   // Reverb / Decay
    }};
    for (size_t i=0; i<knobs.size(); ++i)
    {
        const float diameter = (i < 5 ? 62.0f : 56.0f);
        place (*knobs[i], centres[i].x - diameter*0.5f,
               centres[i].y - diameter*0.5f, diameter, diameter);
    }
}
