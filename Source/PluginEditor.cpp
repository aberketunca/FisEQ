#include "PluginEditor.h"
#include "PluginProcessor.h"



FisEQAudioProcessorEditor::FisEQAudioProcessorEditor(FisEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(600, 400);

    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        70,
        20);

    addAndMakeVisible(gainSlider);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(gainLabel);

    gainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters,
        "gain",
        gainSlider);
}

void FisEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(28.0f);

    g.drawFittedText(
        "FisEQ",
        0,
        20,
        getWidth(),
        40,
        juce::Justification::centred,
        1);
}

void FisEQAudioProcessorEditor::resized()
{
    gainSlider.setBounds(250, 110, 100, 100);
    gainLabel.setBounds(250, 215, 100, 20);
}
