#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include <array>

/**
 * Interactive EQ curve overlay.
 * Draws the composite magnitude response with glow effects,
 * and provides draggable nodes with rich visual feedback.
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

    /** Call to refresh the curve (typically from timer). */
    void updateCurve();

private:
    FisEQAudioProcessor& processor;

    // Frequency-axis helpers (log scale 20-20kHz)
    float frequencyToX(float freq) const;
    float xToFrequency(float normX) const;

    // dB-axis helpers (range: +/-24 dB centered)
    float dbToY(float db) const;
    float yToDb(float y) const;

    // Node positions on screen
    struct BandNode
    {
        juce::Point<float> position;
        juce::Colour colour;
    };

    std::array<BandNode, FisEQAudioProcessor::numBands> nodes;

    int draggedBand = -1;
    int hoveredBand = -1;

    static constexpr float nodeRadius = 8.0f;
    static constexpr float nodeHitRadius = 16.0f;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minDB = -24.0f;
    static constexpr float maxDB = 24.0f;

    // Band colors — vibrant, distinct, modern palette
    static constexpr std::array<juce::uint32, 5> bandColours = {
        0xfff472b6,  // pink
        0xfffbbf24,  // amber
        0xff34d399,  // emerald
        0xff60a5fa,  // blue
        0xffc084fc   // purple
    };

    int findBandAt(juce::Point<float> pos) const;

    void drawPerBandCurve(juce::Graphics& g, int bandIndex, float w, float h);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveEditor)
};
