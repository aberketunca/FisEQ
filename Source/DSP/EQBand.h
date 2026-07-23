#pragma once

#include <juce_dsp/juce_dsp.h>

class EQBand
{
public:
    void prepare(double sampleRate,
                 int samplesPerBlock,
                 int numChannels);

    void update(float frequency,
                float gainDB,
                float q);

    void process(juce::AudioBuffer<float>& buffer);

private:
    double currentSampleRate = 44100.0;

    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > filter;
};