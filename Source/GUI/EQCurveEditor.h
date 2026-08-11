#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../PluginProcessor.h"
#include <array>

/**
 * Interactive EQ curve overlay.
 * Draws the composite magnitude response of all bands,
 * and provides draggable nodes for each band's frequency/gain.
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

    // Frequency-axis helpers (log scale 20–20kHz)
    float frequencyToX(float freq) const;
    float xToFrequency(float normX) const;

    // dB-axis helpers (range: ±24 dB centered)
    float dbToY(float db) const;
    float yToDb(float normY) const;

    // Node positions on screen (recalculated each frame)
    struct BandNode
    {
        juce::Point<float> position;
        juce::Colour colour;
    };

    std::array<BandNode, FisEQAudioProcessor::numBands> nodes;

    int draggedBand = -1;   // -1 = nothing being dragged
    int hoveredBand = -1;   // -1 = no hover

    static constexpr float nodeRadius = 7.0f;
    static constexpr float nodeHitRadius = 14.0f;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minDB = -24.0f;
    static constexpr float maxDB = 24.0f;

    // Band colors
    static constexpr std::array<juce::uint32, 5> bandColours = {
        0xffff6b6b,  // red
        0xfffeca57,  // yellow
        0xff48dbfb,  // cyan
        0xff00d2d3,  // teal
        0xffff9ff3   // pink
    };

    int findBandAt(juce::Point<float> pos) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQCurveEditor)
};
