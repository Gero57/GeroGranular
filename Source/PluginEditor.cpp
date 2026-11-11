#include "PluginEditor.h"

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
    auto b = getLocalBounds().toFloat().reduced (3.0f);
    auto centre = b.getCentre();
    const float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.43f;
    const auto disk = juce::Rectangle<float> (centre.x-radius, centre.y-radius, radius*2, radius*2);

    g.setColour (juce::Colour (0xff080d12));
    g.fillEllipse (disk);
    g.setColour (juce::Colour (0xff58636d));
    g.drawEllipse (disk, 1.5f);
    g.setColour (juce::Colour (0xff1b252d));
    g.drawEllipse (disk.reduced (4.0f), 1.0f);

    const float p = (float) ((getValue()-getMinimum()) /
                     juce::jmax (0.000001, getMaximum()-getMinimum()));
    const float angle = juce::MathConstants<float>::pi*1.2f +
                        p * juce::MathConstants<float>::pi*1.6f;
    juce::Path pointer;
    pointer.startNewSubPath (centre.x, centre.y);
    pointer.lineTo (centre.x + std::cos(angle)*radius*0.72f,
                    centre.y + std::sin(angle)*radius*0.72f);
    g.setColour (juce::Colour (0xff00cfff));
    g.strokePath (pointer, juce::PathStrokeType (2.6f,
                  juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.fillEllipse (centre.x-2.5f, centre.y-2.5f, 5.0f, 5.0f);
}

GeroGrainAudioProcessorEditor::GeroGrainAudioProcessorEditor (GeroGrainAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible (loadButton);
    loadButton.onClick = [this] { chooseSample(); };
    loadButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff27323b));
    loadButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    auto setupEffectButton = [] (juce::TextButton& button)
    {
        button.setClickingTogglesState (true);
        button.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff101820));
        button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff07536b));
        button.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffdceaf2));
        button.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff8beaff));
    };
    setupEffectButton (chorusButton);
    setupEffectButton (delayButton);
    setupEffectButton (reverbButton);
    addAndMakeVisible (chorusButton);
    addAndMakeVisible (delayButton);
    addAndMakeVisible (reverbButton);

    addAndMakeVisible (sampleLabel);
    sampleLabel.setText (processor.getSampleName(), juce::dontSendNotification);
    sampleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8ed));
    sampleLabel.setJustificationType (juce::Justification::centredLeft);
    sampleLabel.setFont (juce::Font (13.0f));

    for (size_t i=0; i<knobCount; ++i)
    {
        knobs[i] = std::make_unique<NeonKnob>();
        addAndMakeVisible (*knobs[i]);
        knobLabels[i] = std::make_unique<juce::Label>();
        knobLabels[i]->setText (names[i], juce::dontSendNotification);
        knobLabels[i]->setJustificationType (juce::Justification::centred);
        knobLabels[i]->setColour (juce::Label::textColourId, juce::Colour (0xffd8e0e7));
        knobLabels[i]->setFont (juce::Font (10.5f, juce::Font::bold));
        addAndMakeVisible (*knobLabels[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (processor.parameters, ids[i], *knobs[i]);
    }
    setSize (1200, 800);
}

void GeroGrainAudioProcessorEditor::chooseSample()
{
    auto chooser = std::make_shared<juce::FileChooser> ("Load audio source", juce::File{},
                                                        "*.wav;*.aif;*.aiff;*.flac;*.ogg");
    chooser->launchAsync (juce::FileBrowserComponent::openMode |
                          juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc)
        {
            if (fc.getResult().existsAsFile() && processor.loadSample (fc.getResult()))
                sampleLabel.setText (processor.getSampleName(), juce::dontSendNotification);
        });
}

void GeroGrainAudioProcessorEditor::paint (juce::Graphics& g)
{
    // The uploaded 1200x800 blue marble panel is the fixed background.
    // All JUCE controls are child components and therefore remain above it.
    static const juce::Image backgroundImage = juce::ImageCache::getFromMemory
        (BinaryData::BaseGrain_png, BinaryData::BaseGrain_pngSize);

    if (backgroundImage.isValid())
        g.drawImage (backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll (juce::Colour (0xff123b68));

    g.setColour (juce::Colour (0xfff5fbff));
    g.setFont (juce::Font (28.0f, juce::Font::bold));
    g.drawText ("GeroGrain", 0, 13, getWidth(), 36, juce::Justification::centred);
    g.setColour (juce::Colour (0xffe0f4ff));
    g.setFont (juce::Font (10.0f));
    g.drawText ("GRANULAR SYNTHESIS", 0, 48, getWidth(), 18, juce::Justification::centred);

    auto sectionTitle = [&g] (int x, int y, int w, const juce::String& title)
    {
        g.setColour (juce::Colour (0xfff3faff).withAlpha (0.96f));
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (title, x, y, w, 18, juce::Justification::centredLeft);
    };

    sectionTitle (28, 93, 390, "SOURCE");
    sectionTitle (460, 93, 700, "GRAIN ENGINE");
    sectionTitle (28, 231, 450, "POSITION");
    sectionTitle (520, 231, 270, "PITCH");
    sectionTitle (832, 231, 330, "ENVELOPE");
    sectionTitle (28, 395, 250, "LFO");
    sectionTitle (320, 395, 250, "MOD MATRIX");
    sectionTitle (612, 395, 250, "FILTER");
    sectionTitle (904, 395, 250, "EFFECTS");

    auto panel = [&g] (int x, int y, int w, int h)
    {
        g.setColour (juce::Colour (0xff071019).withAlpha (0.38f));
        g.fillRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 9.0f);
        g.setColour (juce::Colour (0xffa8bdcc).withAlpha (0.52f));
        g.drawRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 9.0f, 1.0f);
    };
    panel (18, 86, 420, 125);
    panel (450, 86, 732, 125);
    panel (18, 224, 480, 150);
    panel (510, 224, 300, 150);
    panel (822, 224, 360, 150);
    panel (18, 388, 280, 150);
    panel (310, 388, 280, 150);
    panel (602, 388, 280, 150);
    panel (894, 388, 288, 150);

    g.setColour (juce::Colour (0xffeff8ff).withAlpha (0.78f));
    g.setFont (juce::Font (9.0f));
    g.drawText ("EXPANSION AREA", 0, 748, getWidth(), 15, juce::Justification::centred);
}

void GeroGrainAudioProcessorEditor::resized()
{
    loadButton.setBounds (34, 139, 58, 34);
    sampleLabel.setBounds (104, 139, 310, 34);
    chorusButton.setBounds (910, 510, 76, 23);
    delayButton.setBounds (994, 510, 76, 23);
    reverbButton.setBounds (1078, 510, 92, 23);

    struct Group { int first, count, x, y, w; };
    const Group groups[] = {
        {0,6,462,112,708},
        {6,4,30,251,456},
        {10,3,522,251,276},
        {13,4,834,251,336},
        {17,3,28,415,260},
        {20,3,320,415,260},
        {23,3,612,415,260},
        {26,4,902,415,272}
    };

    for (const auto& group : groups)
    {
        const float cell = (float) group.w / (float) group.count;
        for (int j=0; j<group.count; ++j)
        {
            const int i = group.first+j;
            const int cx = group.x + (int)(cell*(j+0.5f));
            knobs[(size_t)i]->setBounds (cx-29, group.y, 58, 58);
            knobLabels[(size_t)i]->setBounds (cx-(int)(cell*0.48f), group.y+59,
                                              (int)(cell*0.96f), 17);
        }
    }
}
