#include "SpectrumAnalyzer.h"
#include <cmath>

SpectrumAnalyzer::SpectrumAnalyzer()
    : fft(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
}

void SpectrumAnalyzer::pushSamples(const float* data, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        fifoBuffer[static_cast<size_t>(fifoIndex)] = data[i];
        ++fifoIndex;

        if (fifoIndex >= fftSize)
        {
            processFFT();
            fifoIndex = 0;
        }
    }
}

void SpectrumAnalyzer::processFFT()
{
    // Copy FIFO into FFT working buffer
    std::copy(fifoBuffer.begin(), fifoBuffer.end(), fftData.begin());

    // Zero the imaginary part
    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    // Apply Hann window
    window.multiplyWithWindowingTable(fftData.data(), fftSize);

    // Perform forward FFT (produces interleaved real/imag)
    fft.performFrequencyOnlyForwardTransform(fftData.data(), true);

    // Convert to dB magnitude
    constexpr float minDB = -100.0f;
    constexpr float maxDB = 0.0f;

    for (int i = 0; i < numBins; ++i)
    {
        float magnitude = fftData[static_cast<size_t>(i)];

        // Convert to dB, clamp
        float db = juce::Decibels::gainToDecibels(magnitude, minDB);
        // Normalize to 0..1 range
        float normalized = juce::jmap(db, minDB, maxDB, 0.0f, 1.0f);
        magnitudeBuffer[static_cast<size_t>(i)] = normalized;
    }

    dataReady.store(true, std::memory_order_release);
}

bool SpectrumAnalyzer::pullMagnitudes(std::array<float, numBins>& output)
{
    if (dataReady.load(std::memory_order_acquire))
    {
        output = magnitudeBuffer;
        dataReady.store(false, std::memory_order_release);
        return true;
    }
    return false;
}

void SpectrumAnalyzer::reset()
{
    fifoBuffer.fill(0.0f);
    fftData.fill(0.0f);
    magnitudeBuffer.fill(0.0f);
    fifoIndex = 0;
    dataReady.store(false);
}
