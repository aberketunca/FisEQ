#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/SpectrumAnalyzer.h"
#include <array>

/**
 * Scientific real-time spectrum analyzer display.
 * Precise dBFS-calibrated rendering with analytical grid,
 * octave-band markers, thin line rendering, and subtle fill.
 * Designed to look like a measurement tool, not a visualizer.
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
    float frequencyToX(float freq) const;
    float xToFrequency(float normX) const;

    /** Map dB value to Y coordinate (scientific scale: 0 dBFS at top, -100 dBFS at bottom). */
    float dbToY(float db, float h) const;

    /** Build spectrum path with dBFS calibration. */
    void buildAnalyzerPath(juce::Path& path, float w, float h,
                           const std::array<float, SpectrumAnalyzer::numBins>& mags) const;

    // Smoothed display magnitudes
    std::array<float, SpectrumAnalyzer::numBins> displayMagnitudes{};

    // Peak hold values (slower decay — thin line)
    std::array<float, SpectrumAnalyzer::numBins> peakMagnitudes{};

    // Raw data
    std::array<float, SpectrumAnalyzer::numBins> rawMagnitudes{};

    double sampleRate = 44100.0;

    // Smoothing
    static constexpr float decayRate = 0.84f;
    static constexpr float riseRate  = 0.92f;
    static constexpr float peakDecay = 0.995f;

    // dB range for display
    static constexpr float maxDBFS = 0.0f;
    static constexpr float minDBFS = -90.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDisplay)
};
