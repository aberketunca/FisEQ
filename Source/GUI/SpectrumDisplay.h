#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/SpectrumAnalyzer.h"
#include <array>

/**
 * Real-time spectrum analyzer display.
 * Multi-layered rendering: filled gradient + glow stroke + peak hold.
 * Logarithmic frequency axis, smooth decay, rich color palette.
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

    /** Build the spectrum path from display magnitudes. */
    void buildSpectrumPath(juce::Path& path, float w, float h, float heightScale) const;

    // Smoothed display magnitudes (what we actually render)
    std::array<float, SpectrumAnalyzer::numBins> displayMagnitudes{};

    // Peak hold values (slower decay)
    std::array<float, SpectrumAnalyzer::numBins> peakMagnitudes{};

    // Raw data from last pull
    std::array<float, SpectrumAnalyzer::numBins> rawMagnitudes{};

    double sampleRate = 44100.0;

    // Smoothing parameters
    static constexpr float decayRate = 0.82f;
    static constexpr float riseRate  = 0.93f;
    static constexpr float peakDecay = 0.992f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};
