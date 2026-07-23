#pragma once

#include <juce_audio_processors/juce_audio_processors.h>


#include "PluginProcessor.h"

class FisEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FisEQAudioProcessorEditor(FisEQAudioProcessor&);
    ~FisEQAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FisEQAudioProcessor& audioProcessor;

    juce::Slider gainSlider;
    juce::Label gainLabel;

    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FisEQAudioProcessorEditor)
};