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
        *filter.state = *newCoeffs;
}

void EQBand::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}

double EQBand::magnitudeForParams(FilterType type,
                                  double sampleRate,
                                  float frequency,
                                  float gainDB,
                                  float q,
                                  double atFrequency)
{
    if (sampleRate <= 0.0)
        return 1.0;

    float gain = juce::Decibels::decibelsToGain(gainDB);

    juce::dsp::IIR::Coefficients<float>::Ptr coeffs;
    switch (type)
    {
        case FilterType::LowShelf:
            coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, frequency, q, gain);
            break;
        case FilterType::Peak:
            coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, frequency, q, gain);
            break;
        case FilterType::HighShelf:
            coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, frequency, q, gain);
            break;
    }

    if (coeffs == nullptr)
        return 1.0;

    return coeffs->getMagnitudeForFrequency(atFrequency, sampleRate);
}
