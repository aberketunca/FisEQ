#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/EQBand.h"
#include "DSP/SpectrumAnalyzer.h"
#include <array>

class FisEQAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numBands = 5;

    FisEQAudioProcessor();
    ~FisEQAudioProcessor() override = default;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState parameters;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout&) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // Public access for GUI
    SpectrumAnalyzer& getAnalyzer() { return analyzer; }
    EQBand& getBand(int index) { return bands[static_cast<size_t>(index)]; }

    /** Compute the composite magnitude response of all bands at a given frequency. */
    double getCompositeMagnitude(double frequency) const;

private:
    std::array<EQBand, numBands> bands;
    SpectrumAnalyzer analyzer;

    // Band filter types (fixed topology)
    static constexpr std::array<FilterType, numBands> bandTypes = {
        FilterType::LowShelf,
        FilterType::Peak,
        FilterType::Peak,
        FilterType::Peak,
        FilterType::HighShelf
    };

    // Default frequencies per band
    static constexpr std::array<float, numBands> defaultFrequencies = {
        80.0f, 300.0f, 1000.0f, 4000.0f, 12000.0f
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FisEQAudioProcessor)
};
