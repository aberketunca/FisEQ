#include "EQBand.h"

void EQBand::prepare(double sampleRate,
                     int samplesPerBlock,
                     int numChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = static_cast<uint32_t>(numChannels);

    filter.prepare(spec);
}

void EQBand::update(float frequency,
                    float gainDB,
                    float q)
{
    *filter.state =
        *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            currentSampleRate,
            frequency,
            q,
            juce::Decibels::decibelsToGain(gainDB));
}

void EQBand::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    filter.process(context);
}