#include "PluginEditor.h"
#include "BinaryData.h"

NeonKnob::NeonKnob()
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::TextBoxBelow, false, 66, 18);
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00dfff));
    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff153c49));
    setColour (juce::Slider::thumbColourId, juce::Colour(0xffe9ffff));
    setColour (juce::Slider::textBoxTextColourId, juce::Colour(0xffd8fbff));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}
void NeonKnob::paint (juce::Graphics& g)
{
    juce::Slider::paint(g);
    auto b = getLocalBounds().toFloat().reduced(5.0f);
    auto c = b.getCentre();
    g.setColour(juce::Colour(0x6600dfff));
    g.drawEllipse(b, 1.5f);
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
    sampleLabel.setJustificationType(juce::Justification::centred);
    for (size_t i=0; i<knobs.size(); ++i)
    {
        knobs[i] = std::make_unique<NeonKnob>();
        addAndMakeVisible(*knobs[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, ids[i], *knobs[i]);
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
    if (background.isValid()) g.drawImage(background, getLocalBounds().toFloat());
    else { g.fillAll(juce::Colour(0xff071018)); g.setColour(juce::Colours::white); g.drawText("GeroGrain", getLocalBounds(), juce::Justification::centred); }
}
void GeroGrainAudioProcessorEditor::resized()
{
    const float sx = getWidth()/1200.0f, sy = getHeight()/800.0f;
    auto place = [sx,sy](juce::Component& c,float x,float y,float w,float h){ c.setBounds((int)(x*sx),(int)(y*sy),(int)(w*sx),(int)(h*sy)); };
    place(loadButton, 54, 126, 120, 34);
    place(sampleLabel, 176, 126, 285, 34);
    const std::array<juce::Rectangle<float>,11> boxes {{
      {842,145,105,105},{970,145,105,105},{1055,310,90,90},{842,290,105,105},
      {62,480,95,95},{205,480,95,95},{565,585,70,70},{660,585,70,70},
      {970,585,80,80},{1070,585,80,80},{1220,585,80,80}
    }};
    for (size_t i=0;i<knobs.size();++i)
    {
        auto r=boxes[i]; place(*knobs[i],r.getX(),r.getY(),r.getWidth(),r.getHeight());
    }
}
