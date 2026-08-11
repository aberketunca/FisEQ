#include "EQBand.h"

void EQBand::prepare(double sampleRate,
                     int samplesPerBlock,
                     int numChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    filter.prepare(spec);
}

void EQBand::update(float frequency,
                    float gainDB,
                    float q,
                    FilterType type)
{
    if (currentSampleRate <= 0.0)
        return;

    float gain = juce::Decibels::decibelsToGain(gainDB);

    juce::dsp::IIR::Coefficients<float>::Ptr newCoeffs;

    switch (type)
    {
        case FilterType::LowShelf:
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                currentSampleRate, frequency, q, gain);
            break;

        case FilterType::Peak:
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, frequency, q, gain);
            break;

        case FilterType::HighShelf:
            newCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                currentSampleRate, frequency, q, gain);
            break;
    }

    if (newCoeffs != nullptr)
    {
        *filter.state = *newCoeffs;
        currentCoefficients = newCoeffs;
    }
}

void EQBand::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}

double EQBand::getMagnitudeAtFrequency(double frequency) const
{
    if (bypassed || currentCoefficients == nullptr)
        return 1.0;

    return currentCoefficients->getMagnitudeForFrequency(frequency, currentSampleRate);
}
