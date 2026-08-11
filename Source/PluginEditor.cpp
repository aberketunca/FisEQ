#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>
#include <cmath>

FisEQAudioProcessorEditor::FisEQAudioProcessorEditor(FisEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      eqCurveEditor(p)
{
    setSize(900, 500);
    setResizable(true, true);
    setResizeLimits(600, 350, 1600, 900);

    addAndMakeVisible(spectrumDisplay);
    addAndMakeVisible(eqCurveEditor);

    spectrumDisplay.setSampleRate(audioProcessor.getSampleRate());

    startTimerHz(60);
}

FisEQAudioProcessorEditor::~FisEQAudioProcessorEditor()
{
    stopTimer();
}

void FisEQAudioProcessorEditor::timerCallback()
{
    spectrumDisplay.setSampleRate(audioProcessor.getSampleRate());
    spectrumDisplay.updateSpectrum(audioProcessor.getAnalyzer());
    eqCurveEditor.updateCurve();
}

void FisEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark background
    g.fillAll(juce::Colour(0xff0a0e17));

    auto bounds = getLocalBounds();

    // Top bar with logo
    auto topBar = bounds.removeFromTop(44);
    g.setColour(juce::Colour(0xff0d1220));
    g.fillRect(topBar);

    // Logo
    auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MAROLA___TTF,
        BinaryData::MAROLA___TTFSize);

    juce::Font logoFont(juce::FontOptions(typeface).withHeight(28.0f));
    g.setFont(logoFont);
    g.setColour(juce::Colour(0xff00d4ff));
    g.drawText("FisEQ", topBar.reduced(16, 0), juce::Justification::centredLeft);

    // Subtitle
    g.setFont(12.0f);
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText("5-Band Parametric EQ", topBar.reduced(16, 0), juce::Justification::centredRight);

    // Bottom area for frequency labels
    auto bottomBar = bounds.removeFromBottom(22);
    drawFrequencyLabels(g, bottomBar);

    // Left area for dB labels
    auto leftBar = bounds.removeFromLeft(32);
    drawDBLabels(g, leftBar);

    // Graph border
    auto graphArea = bounds.reduced(1);
    g.setColour(juce::Colour(0xff1a2035));
    g.drawRect(graphArea, 1);
}

void FisEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Remove top bar
    bounds.removeFromTop(44);
    // Remove bottom label area
    bounds.removeFromBottom(22);
    // Remove left dB label area
    bounds.removeFromLeft(32);

    auto graphArea = bounds.reduced(1);

    // Both components overlap the same area (spectrogram behind, curve on top)
    spectrumDisplay.setBounds(graphArea);
    eqCurveEditor.setBounds(graphArea);
}

void FisEQAudioProcessorEditor::drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(10.0f);
    g.setColour(juce::Colours::white.withAlpha(0.45f));

    const float freqs[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    const char* labels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

    float graphLeft = static_cast<float>(area.getX() + 32); // offset for dB label area
    float graphWidth = static_cast<float>(area.getWidth() - 32);

    for (int i = 0; i < 10; ++i)
    {
        float normX = std::log10(freqs[i] / 20.0f) / std::log10(20000.0f / 20.0f);
        float x = graphLeft + normX * graphWidth;

        g.drawText(labels[i],
                   static_cast<int>(x - 15), area.getY(),
                   30, area.getHeight(),
                   juce::Justification::centred);
    }
}

void FisEQAudioProcessorEditor::drawDBLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(9.0f);
    g.setColour(juce::Colours::white.withAlpha(0.4f));

    float h = static_cast<float>(area.getHeight());

    const float dbs[] = { 24, 12, 0, -12, -24 };

    for (float db : dbs)
    {
        // Map dB to Y: 0dB at center, +24 at top, -24 at bottom
        float normY = 0.5f - (db / 24.0f) * 0.5f;
        int y = area.getY() + static_cast<int>(normY * h);

        juce::String label;
        if (db > 0)
            label = "+" + juce::String(static_cast<int>(db));
        else
            label = juce::String(static_cast<int>(db));

        g.drawText(label,
                   area.getX(), y - 6,
                   area.getWidth() - 2, 12,
                   juce::Justification::centredRight);
    }
}
