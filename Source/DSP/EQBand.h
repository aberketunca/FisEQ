#pragma once

#include <juce_dsp/juce_dsp.h>

enum class FilterType
{
    LowShelf,
    Peak,
    HighShelf
};

class EQBand
{
public:
    void prepare(double sampleRate,
                 int samplesPerBlock,
                 int numChannels);

    void update(float frequency,
                float gainDB,
                float q,
                FilterType type);

    void process(juce::AudioBuffer<float>& buffer);

    /** Compute magnitude response at a given frequency (for curve drawing). */
    double getMagnitudeAtFrequency(double frequency) const;

    bool isBypassed() const { return bypassed; }
    void setBypassed(bool b) { bypassed = b; }

private:
    double currentSampleRate = 44100.0;
    bool bypassed = false;

    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > filter;

    // Keep a copy of current coefficients for magnitude response queries
    juce::dsp::IIR::Coefficients<float>::Ptr currentCoefficients;
};
