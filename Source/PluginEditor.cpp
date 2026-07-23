#include "PluginEditor.h"
#include "PluginProcessor.h"

FisEQAudioProcessorEditor::FisEQAudioProcessorEditor(FisEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(600, 400);
}

void FisEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(28.0f);

    g.drawFittedText(
        "FisEQ",
        getLocalBounds(),
        juce::Justification::centred,
        1);
}

void FisEQAudioProcessorEditor::resized()
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FisEQAudioProcessor();
}