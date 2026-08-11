#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/SpectrumDisplay.h"
#include "GUI/EQCurveEditor.h"

class FisEQAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    FisEQAudioProcessorEditor(FisEQAudioProcessor&);
    ~FisEQAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    FisEQAudioProcessor& audioProcessor;

    SpectrumDisplay spectrumDisplay;
    EQCurveEditor eqCurveEditor;

    void drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> area);
    void drawDBLabels(juce::Graphics& g, juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FisEQAudioProcessorEditor)
};
