#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>

/**
 * Lock-free STFT spectrum analyzer.
 * Audio thread pushes samples into a FIFO; GUI thread pulls magnitude data.
 */
class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 11;                        // 2048-point FFT
    static constexpr int fftSize = 1 << fftOrder;             // 2048
    static constexpr int numBins = fftSize / 2 + 1;           // 1025 bins

    SpectrumAnalyzer();

    /** Call from audio thread — pushes samples into the FIFO. */
    void pushSamples(const float* data, int numSamples);

    /** Call from GUI thread — returns true if new spectrum data is available. */
    bool pullMagnitudes(std::array<float, numBins>& output);

    /** Reset internal state. */
    void reset();

private:
    void processFFT();

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    // FIFO ring buffer (audio thread writes, internal consumption)
    std::array<float, fftSize> fifoBuffer{};
    int fifoIndex = 0;

    // FFT working buffers
    std::array<float, fftSize * 2> fftData{};

    // Result buffer — written after FFT, read by GUI
    std::array<float, numBins> magnitudeBuffer{};
    std::atomic<bool> dataReady{ false };
};
