#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include <array>

/**
 * Interactive EQ curve overlay with Pro-Q style interaction:
 * - Draggable nodes (freq/gain)
 * - Scroll-wheel Q adjustment
 * - Floating parameter tooltip
 * - Double-click to reset
 * - Visual bandwidth (Q) indicator
 */
class EQCurveEditor : public juce::Component
{
public:
    EQCurveEditor(FisEQAudioProcessor& processor);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    /** Call to refresh the curve (typically from timer). */
    void updateCurve();

private:
    FisEQAudioProcessor& processor;

    float frequencyToX(float freq) const;
    float xToFrequency(float normX) const;
    float dbToY(float db) const;
    float yToDb(float y) const;

    struct BandNode
    {
        juce::Point<float> position;
        juce::Colour colour;
    };

    std::array<BandNode, FisEQAudioProcessor::numBands> nodes;

    int draggedBand = -1;
    int hoveredBand = -1;

    // Tooltip state
    bool showTooltip = false;
    juce::Point<float> tooltipPos;
    float tooltipFreq = 0.0f;
    float tooltipGain = 0.0f;
    float tooltipQ = 0.0f;
    int tooltipBand = -1;

    static constexpr float nodeRadius = 8.0f;
    static constexpr float nodeHitRadius = 16.0f;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minDB = -24.0f;
    static constexpr float maxDB = 24.0f;

    static constexpr std::array<juce::uint32, 5> bandColours = {
        0xfff472b6,  // pink
        0xfffbbf24,  // amber
        0xff34d399,  // emerald
        0xff60a5fa,  // blue
        0xffc084fc   // purple
    };

    int findBandAt(juce::Point<float> pos) const;
    void drawPerBandCurve(juce::Graphics& g, int bandIndex, float w, float h);
    void drawBandwidthIndicator(juce::Graphics& g, int bandIndex, float w, float h);
    void drawTooltip(juce::Graphics& g);

    /** Format frequency intelligently (Hz vs kHz). */
    static juce::String formatFrequency(float freq);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveEditor)
};
