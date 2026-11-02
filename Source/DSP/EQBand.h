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

    /**
     * Thread-safe magnitude response for curve drawing.
     *
     * Builds a throwaway coefficient set from the supplied parameter values
     * on the calling (GUI) thread, so it never touches the audio thread's
     * live filter state. Static on purpose — no shared mutable state.
     */
    static double magnitudeForParams(FilterType type,
                                     double sampleRate,
                                     float frequency,
                                     float gainDB,
                                     float q,
                                     double atFrequency);

    bool isBypassed() const { return bypassed; }
    void setBypassed(bool b) { bypassed = b; }

private:
    double currentSampleRate = 44100.0;
    bool bypassed = false;

    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > filter;
};
