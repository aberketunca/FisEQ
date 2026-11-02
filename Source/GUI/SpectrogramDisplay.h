#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../DSP/SpectrumAnalyzer.h"
#include <array>

/**
 * Scrolling spectrogram heatmap with a vintage 1970s aesthetic.
 *
 * Each incoming FFT frame becomes a new column on the right edge; the
 * image scrolls left over time. Frequency is mapped logarithmically on
 * the vertical axis (low at bottom, high at top). Magnitude is mapped
 * through a warm, gritty film-inspired colour ramp.
 */
class SpectrogramDisplay : public juce::Component
{
public:
    SpectrogramDisplay();

    /** Call from a timer: pulls the latest frame and scrolls it in. */
    void updateSpectrum(SpectrumAnalyzer& analyzer);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setSampleRate(double sr) { sampleRate = sr; }

private:
    /** Vintage colour ramp: value in 0..1 -> warm film colour. */
    juce::Colour mapColour(float value) const;

    /** Push one FFT frame as a new scrolling column. */
    void pushColumn(const std::array<float, SpectrumAnalyzer::numBins>& mags);

    /** Rebuild the colour lookup table. */
    void buildColourLUT();

    juce::Image spectrogramImage;
    std::array<float, SpectrumAnalyzer::numBins> frame{};

    double sampleRate = 44100.0;

    // Precomputed colour ramp (256 entries)
    static constexpr int lutSize = 256;
    std::array<juce::Colour, lutSize> colourLUT;

    // Film-grain seed state
    juce::Random grainRandom;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;

    bool imageReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramDisplay)
};
