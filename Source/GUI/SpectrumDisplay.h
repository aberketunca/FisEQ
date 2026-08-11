#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/SpectrumAnalyzer.h"
#include <array>

/**
 * Real-time spectrum analyzer display.
 * Renders FFT magnitude data as a filled gradient spectrum
 * with logarithmic frequency axis and smooth decay.
 */
class SpectrumDisplay : public juce::Component
{
public:
    SpectrumDisplay();

    /** Call from a timer to pull new data from the analyzer. */
    void updateSpectrum(SpectrumAnalyzer& analyzer);

    void paint(juce::Graphics& g) override;
    void resized() override;

    /** Set the sample rate for accurate frequency mapping. */
    void setSampleRate(double sr) { sampleRate = sr; }

private:
    /** Map a frequency (Hz) to an X position (0..1) on log scale. */
    float frequencyToX(float freq) const;

    /** Map an X position (0..1) back to frequency (Hz). */
    float xToFrequency(float normX) const;

    // Smoothed display magnitudes (what we actually render)
    std::array<float, SpectrumAnalyzer::numBins> displayMagnitudes{};

    // Raw data from last pull
    std::array<float, SpectrumAnalyzer::numBins> rawMagnitudes{};

    double sampleRate = 44100.0;

    // Smoothing factor (0 = no smoothing, 1 = infinite hold)
    static constexpr float decayRate = 0.78f;
    static constexpr float riseRate  = 0.92f;

    // Path cache for the spectrum fill
    juce::Path spectrumPath;

    // Gradient colors
    juce::Colour bottomColour{ 0xff0a1628 };   // dark navy
    juce::Colour midColour{ 0xff1640C9 };      // blue
    juce::Colour topColour{ 0xff00d4ff };       // cyan

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};
