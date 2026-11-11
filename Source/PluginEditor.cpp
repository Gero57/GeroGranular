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
    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient metal (juce::Colour (0xff30363c), 0, 0,
                                juce::Colour (0xff11161b), 0, (float)getHeight(), false);
    metal.addColour (0.35, juce::Colour (0xff20272e));
    metal.addColour (0.70, juce::Colour (0xff171d23));
    g.setGradientFill (metal);
    g.fillRoundedRectangle (bounds.reduced (3.0f), 13.0f);

    g.setColour (juce::Colour (0xff59636d));
    g.drawRoundedRectangle (bounds.reduced (4.0f), 12.0f, 1.2f);

    // Four fixed metal screws at the panel corners.
    auto screw = [&g] (float x, float y)
    {
        g.setColour (juce::Colour (0xff080b0e));
        g.fillEllipse (x-10.0f, y-10.0f, 20.0f, 20.0f);
        g.setColour (juce::Colour (0xff89939b));
        g.drawEllipse (x-9.0f, y-9.0f, 18.0f, 18.0f, 1.2f);
        g.setColour (juce::Colour (0xff3d464e));
        g.fillEllipse (x-6.5f, y-6.5f, 13.0f, 13.0f);
        g.setColour (juce::Colour (0xffc0c8ce));
        g.drawLine (x-3.5f, y+3.5f, x+3.5f, y-3.5f, 1.8f);
    };
    screw (20.0f, 20.0f);
    screw ((float)getWidth()-20.0f, 20.0f);
    screw (20.0f, (float)getHeight()-20.0f);
    screw ((float)getWidth()-20.0f, (float)getHeight()-20.0f);

    // Subtle brushed-metal grain, entirely behind the controls.
    g.setColour (juce::Colour (0xffd5dde4).withAlpha (0.025f));
    for (int y=6; y<getHeight()-6; y+=5)
        g.drawHorizontalLine (y, 8.0f, (float)getWidth()-8.0f);

    g.setColour (juce::Colour (0xfff0f3f6));
    g.setFont (juce::Font (28.0f, juce::Font::bold));
    g.drawText ("GeroGrain", 0, 13, getWidth(), 36, juce::Justification::centred);
    g.setColour (juce::Colour (0xffaeb9c2));
    g.setFont (juce::Font (10.0f));
    g.drawText ("GRANULAR SYNTHESIS", 0, 48, getWidth(), 18, juce::Justification::centred);

    auto section = [&g] (juce::Rectangle<int> r, const juce::String& title)
    {
        g.setColour (juce::Colour (0xff0c1116).withAlpha (0.55f));
        g.fillRoundedRectangle (r.toFloat(), 9.0f);
        g.setColour (juce::Colour (0xff66717b));
        g.drawRoundedRectangle (r.toFloat(), 9.0f, 1.0f);
        g.setColour (juce::Colour (0xffdce4ea));
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (title, r.getX()+12, r.getY()+7, r.getWidth()-24, 18,
                    juce::Justification::centredLeft);
    };

    section ({18, 86, 420, 125}, "SOURCE");
    section ({450, 86, 732, 125}, "GRAIN ENGINE");
    section ({18, 224, 480, 150}, "POSITION");
    section ({510, 224, 300, 150}, "PITCH");
    section ({822, 224, 360, 150}, "ENVELOPE");
    section ({18, 388, 280, 150}, "LFO");
    section ({310, 388, 280, 150}, "MOD MATRIX");
    section ({602, 388, 280, 150}, "FILTER");
    section ({894, 388, 288, 150}, "EFFECTS");

    // Fascia inferiore volutamente libera per espansioni future.
    g.setColour (juce::Colour (0xff9aa5ae).withAlpha (0.22f));
    g.drawLine (20.0f, 565.0f, (float)getWidth()-20.0f, 565.0f, 0.8f);
    g.setColour (juce::Colour (0xff8c969e).withAlpha (0.45f));
    g.setFont (juce::Font (9.0f));
    g.drawText ("EXPANSION AREA", 0, 748, getWidth(), 15, juce::Justification::centred);
}

void GeroGrainAudioProcessorEditor::resized()
{
    loadButton.setBounds (34, 139, 58, 34);
    sampleLabel.setBounds (104, 139, 310, 34);

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
