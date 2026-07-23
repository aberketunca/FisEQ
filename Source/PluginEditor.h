#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class FisEQAudioProcessor;

class FisEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit FisEQAudioProcessorEditor(FisEQAudioProcessor&);
    ~FisEQAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FisEQAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FisEQAudioProcessorEditor)
};