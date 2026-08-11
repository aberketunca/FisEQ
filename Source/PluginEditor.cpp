#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>
#include <cmath>

FisEQAudioProcessorEditor::FisEQAudioProcessorEditor(FisEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      eqCurveEditor(p)
{
    setSize(960, 540);
    setResizable(true, true);
    setResizeLimits(640, 380, 1920, 1080);

    addAndMakeVisible(spectrumDisplay);
    addAndMakeVisible(eqCurveEditor);
    addAndMakeVisible(gainKnob);

    spectrumDisplay.setSampleRate(audioProcessor.getSampleRate());

    gainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters, "output_gain", gainKnob.getSlider());

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
    auto bounds = getLocalBounds();
    float w = static_cast<float>(bounds.getWidth());
    float h = static_cast<float>(bounds.getHeight());

    // === Rich dark background with subtle radial vignette ===
    g.fillAll(juce::Colour(0xff080c14));

    // Subtle radial gradient emanating from center — gives depth
    {
        juce::ColourGradient vignette(
            juce::Colour(0xff101828), w * 0.5f, h * 0.45f,
            juce::Colour(0xff050810), 0.0f, h,
            true); // radial
        g.setGradientFill(vignette);
        g.fillRect(bounds);
    }

    // === Top bar ===
    auto topBar = bounds.removeFromTop(48);

    // Top bar gradient
    {
        juce::ColourGradient topGrad(
            juce::Colour(0xff0f1629), 0.0f, static_cast<float>(topBar.getY()),
            juce::Colour(0xff080c14), 0.0f, static_cast<float>(topBar.getBottom()),
            false);
        g.setGradientFill(topGrad);
        g.fillRect(topBar);
    }

    // Separator line under top bar
    g.setColour(juce::Colour(0xff1e293b));
    g.drawHorizontalLine(topBar.getBottom() - 1, 0.0f, w);

    // Logo
    auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MAROLA___TTF,
        BinaryData::MAROLA___TTFSize);

    juce::Font logoFont(juce::FontOptions(typeface).withHeight(30.0f));
    g.setFont(logoFont);

    // Logo with subtle glow effect
    g.setColour(juce::Colour(0xff06b6d4).withAlpha(0.3f));
    g.drawText("FisEQ", topBar.reduced(18, 0).translated(1, 1),
               juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff22d3ee));
    g.drawText("FisEQ", topBar.reduced(18, 0),
               juce::Justification::centredLeft);

    // Subtitle
    g.setFont(11.0f);
    g.setColour(juce::Colour(0xff64748b));
    g.drawText("5-BAND PARAMETRIC EQ", topBar.reduced(18, 0),
               juce::Justification::centredRight);

    // === Bottom area for frequency labels ===
    auto bottomBar = bounds.removeFromBottom(24);
    drawFrequencyLabels(g, bottomBar);

    // Bottom separator
    g.setColour(juce::Colour(0xff1e293b));
    g.drawHorizontalLine(bottomBar.getY(), 0.0f, w);

    // === Left area for dB labels ===
    auto leftBar = bounds.removeFromLeft(36);
    drawDBLabels(g, leftBar);

    // Left separator
    g.setColour(juce::Colour(0xff1e293b));
    g.drawVerticalLine(leftBar.getRight(), static_cast<float>(leftBar.getY()),
                       static_cast<float>(leftBar.getBottom()));

    // === Graph area subtle inner shadow ===
    auto graphArea = bounds.reduced(1);
    {
        // Top inner shadow
        juce::ColourGradient topShadow(
            juce::Colour(0x15000000), 0.0f, static_cast<float>(graphArea.getY()),
            juce::Colours::transparentBlack, 0.0f, static_cast<float>(graphArea.getY() + 20),
            false);
        g.setGradientFill(topShadow);
        g.fillRect(graphArea.getX(), graphArea.getY(), graphArea.getWidth(), 20);
    }
}

void FisEQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto topBar = bounds.removeFromTop(48);
    bounds.removeFromBottom(24);
    bounds.removeFromLeft(36);

    // Gain knob in the top bar, right-of-center area
    auto knobArea = topBar.removeFromRight(70).reduced(2, 2);
    gainKnob.setBounds(knobArea);

    auto graphArea = bounds.reduced(1);

    spectrumDisplay.setBounds(graphArea);
    eqCurveEditor.setBounds(graphArea);
}

void FisEQAudioProcessorEditor::drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(9.5f);

    const float freqs[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    const char* labels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

    float graphLeft = static_cast<float>(area.getX() + 36);
    float graphWidth = static_cast<float>(area.getWidth() - 36);

    for (int i = 0; i < 10; ++i)
    {
        float normX = std::log10(freqs[i] / 20.0f) / std::log10(20000.0f / 20.0f);
        float x = graphLeft + normX * graphWidth;

        // Emphasize decade markers
        bool isMajor = (i == 2 || i == 5 || i == 8); // 100, 1k, 10k
        g.setColour(juce::Colour(isMajor ? 0xff94a3b8 : 0xff475569));

        g.drawText(labels[i],
                   static_cast<int>(x - 16), area.getY() + 4,
                   32, area.getHeight() - 4,
                   juce::Justification::centred);
    }
}

void FisEQAudioProcessorEditor::drawDBLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(9.0f);

    float h = static_cast<float>(area.getHeight());

    const float dbs[] = { 24, 12, 0, -12, -24 };

    for (float db : dbs)
    {
        float normY = 0.5f - (db / 24.0f) * 0.5f;
        int y = area.getY() + static_cast<int>(normY * h);

        bool isZero = (static_cast<int>(db) == 0);
        g.setColour(juce::Colour(isZero ? 0xff94a3b8 : 0xff475569));

        juce::String label;
        if (db > 0)
            label = "+" + juce::String(static_cast<int>(db));
        else if (isZero)
            label = "0 dB";
        else
            label = juce::String(static_cast<int>(db));

        g.drawText(label,
                   area.getX() + 2, y - 6,
                   area.getWidth() - 4, 12,
                   juce::Justification::centredRight);
    }
}
