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

    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    //================ Sliders ================
    juce::Slider frequencySlider;
    juce::Slider gainSlider;
    juce::Slider qSlider;

    //================ Labels =================
    juce::Label frequencyLabel;
    juce::Label gainLabel;
    juce::Label qLabel;

    //============= Attachments ===============
    std::unique_ptr<SliderAttachment> frequencyAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> qAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FisEQAudioProcessorEditor)
};