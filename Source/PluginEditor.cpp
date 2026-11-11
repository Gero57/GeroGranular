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
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
        (processor.parameters, "reverbon", reverbButton);
    setupEffectButton (velocityButton);
    addAndMakeVisible (velocityButton);
    velocityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
        (processor.parameters, "velocity", velocityButton);
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
    // Fixed 1200 x 800 black lacquer / dark wood chassis.
    // Controls remain separate JUCE child components above this background.
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient base (juce::Colour (0xff202124), 0.0f, 0.0f,
                               juce::Colour (0xff080a0d), 0.0f, (float)getHeight(), false);
    base.addColour (0.24, juce::Colour (0xff191a1c));
    base.addColour (0.58, juce::Colour (0xff111316));
    base.addColour (0.82, juce::Colour (0xff0b0d10));
    g.setGradientFill (base);
    g.fillAll();

    // Fine horizontal irregular streaks: subtle black wood grain, not busy.
    for (int i=0; i<190; ++i)
    {
        const float y = 24.0f + (float)((i * 47) % 748);
        const float x1 = 18.0f + (float)((i * 83) % 270);
        const float x2 = juce::jmin ((float)getWidth()-18.0f, x1 + 180.0f + (float)((i*31)%520));
        const float bend = std::sin ((float)i * 1.71f) * 1.6f;
        const float alpha = (i % 5 == 0) ? 0.040f : 0.018f;
        g.setColour (juce::Colour (i % 3 == 0 ? 0xffb9c3cc : 0xff64717c).withAlpha (alpha));
        g.drawLine (x1, y, x2, y + bend, (i % 7 == 0) ? 1.3f : 0.7f);
    }

    // Soft cool reflections on the lacquer.
    juce::ColourGradient upperGlint (juce::Colour (0xffd6e8f5).withAlpha (0.16f), 42.0f, 100.0f,
                                     juce::Colour (0xff9dbbd0).withAlpha (0.0f), 600.0f, 18.0f, false);
    g.setGradientFill (upperGlint);
    g.drawLine (35.0f, 105.0f, 555.0f, 18.0f, 7.0f);
    juce::ColourGradient lowerGlint (juce::Colour (0xff9bbbd2).withAlpha (0.0f), 640.0f, 790.0f,
                                     juce::Colour (0xffc5d9e8).withAlpha (0.10f), 1160.0f, 690.0f, false);
    g.setGradientFill (lowerGlint);
    g.drawLine (650.0f, 790.0f, 1160.0f, 690.0f, 5.0f);

    g.setColour (juce::Colour (0xff87939d));
    g.drawRoundedRectangle (bounds.reduced (2.0f), 20.0f, 2.0f);
    g.setColour (juce::Colour (0xff05070a));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 16.0f, 4.0f);
    g.setColour (juce::Colour (0xff59636d).withAlpha (0.78f));
    g.drawRoundedRectangle (bounds.reduced (14.0f), 13.0f, 1.0f);

    auto screw = [&g] (float x, float y)
    {
        g.setColour (juce::Colour (0xff050608));
        g.fillEllipse (x-10.0f, y-10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xffa5afb7));
        g.drawEllipse (x-9.0f, y-9.0f, 18.0f, 18.0f, 1.0f);
        g.setColour (juce::Colour (0xff3d454c));
        g.fillEllipse (x-6.5f, y-6.5f, 13.0f, 13.0f);
        g.setColour (juce::Colour (0xffd9e1e7));
        g.drawLine (x-3.5f, y+3.5f, x+3.5f, y-3.5f, 1.5f);
    };
    screw (22.0f,22.0f); screw ((float)getWidth()-22.0f,22.0f);
    screw (22.0f,(float)getHeight()-22.0f);
    screw ((float)getWidth()-22.0f,(float)getHeight()-22.0f);

    g.setColour (juce::Colour (0xfff1f4f6));
    g.setFont (juce::Font (28.0f, juce::Font::bold));
    g.drawText ("GeroGrain", 0, 13, getWidth(), 36, juce::Justification::centred);
    g.setColour (juce::Colour (0xffc7d0d7));
    g.setFont (juce::Font (10.0f));
    g.drawText ("GRANULAR SYNTHESIS", 0, 48, getWidth(), 18, juce::Justification::centred);

    auto panel = [&g] (int x, int y, int w, int h, const juce::String& title)
    {
        g.setColour (juce::Colour (0xff080b0f).withAlpha (0.66f));
        g.fillRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 9.0f);
        g.setColour (juce::Colour (0xff8998a3).withAlpha (0.68f));
        g.drawRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 9.0f, 1.0f);
        g.setColour (juce::Colour (0xffe3e8ec));
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (title, x+12, y+7, w-24, 18, juce::Justification::centredLeft);
    };
    panel (18,86,420,125,"SOURCE");
    panel (450,86,732,125,"GRAIN ENGINE");
    panel (18,224,480,150,"POSITION");
    panel (510,224,300,150,"PITCH");
    panel (822,224,360,150,"ENVELOPE");
    panel (18,388,280,150,"LFO");
    panel (310,388,280,150,"MOD MATRIX");
    panel (602,388,280,150,"FILTER");
    panel (894,388,288,150,"EFFECTS");

    g.setColour (juce::Colour (0xffaeb8c0).withAlpha (0.52f));
    g.drawLine (20.0f,560.0f,(float)getWidth()-20.0f,560.0f,0.8f);
    g.setFont (juce::Font (9.0f));
    g.drawText ("EXPANSION AREA",0,748,getWidth(),15,juce::Justification::centred);
}

void GeroGrainAudioProcessorEditor::resized()
{
    loadButton.setBounds (34, 139, 58, 34);
    sampleLabel.setBounds (104, 139, 310, 34);
    chorusButton.setBounds (910, 510, 76, 23);
    delayButton.setBounds (994, 510, 76, 23);
    reverbButton.setBounds (1078, 510, 92, 23);
    velocityButton.setBounds (34, 578, 120, 28);

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
