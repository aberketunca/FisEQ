#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "GUI/Theme.h"
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
    addAndMakeVisible(spectrogramDisplay);
    addAndMakeVisible(eqCurveEditor);
    addAndMakeVisible(gainKnob);

    spectrumDisplay.setSampleRate(audioProcessor.getSampleRate());
    spectrogramDisplay.setSampleRate(audioProcessor.getSampleRate());

    // View toggle button
    viewToggleButton.setButtonText("SPECTROGRAM");
    viewToggleButton.setClickingTogglesState(true);
    viewToggleButton.onClick = [this]
    {
        setViewMode(viewToggleButton.getToggleState()
                        ? ViewMode::Spectrogram
                        : ViewMode::EQ);
    };
    // Vintage-tinted button colours
    viewToggleButton.setColour(juce::TextButton::buttonColourId,    Theme::col(Theme::bgPanel));
    viewToggleButton.setColour(juce::TextButton::buttonOnColourId,  Theme::col(Theme::accentDeep));
    viewToggleButton.setColour(juce::TextButton::textColourOffId,   Theme::col(Theme::textMid));
    viewToggleButton.setColour(juce::TextButton::textColourOnId,    Theme::col(Theme::textBright));
    addAndMakeVisible(viewToggleButton);

    gainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters, "output_gain", gainKnob.getSlider());

    setViewMode(ViewMode::EQ);

    startTimerHz(60);
}

FisEQAudioProcessorEditor::~FisEQAudioProcessorEditor()
{
    stopTimer();
}

void FisEQAudioProcessorEditor::timerCallback()
{
    double sr = audioProcessor.getSampleRate();

    if (viewMode == ViewMode::EQ)
    {
        spectrumDisplay.setSampleRate(sr);
        spectrumDisplay.updateSpectrum(audioProcessor.getAnalyzer());
        eqCurveEditor.updateCurve();
    }
    else
    {
        spectrogramDisplay.setSampleRate(sr);
        spectrogramDisplay.updateSpectrum(audioProcessor.getAnalyzer());
    }
}

void FisEQAudioProcessorEditor::setViewMode(ViewMode mode)
{
    viewMode = mode;

    bool eq = (mode == ViewMode::EQ);

    spectrumDisplay.setVisible(eq);
    eqCurveEditor.setVisible(eq);
    spectrogramDisplay.setVisible(!eq);

    viewToggleButton.setToggleState(!eq, juce::dontSendNotification);
    viewToggleButton.setButtonText(eq ? "SPECTROGRAM" : "EQ VIEW");

    repaint();
}

void FisEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    float w = static_cast<float>(bounds.getWidth());
    float h = static_cast<float>(bounds.getHeight());

    // === Warm vintage background with radial vignette ===
    g.fillAll(Theme::col(Theme::bgBase));

    {
        juce::ColourGradient vignette(
            Theme::col(Theme::bgRaised), w * 0.5f, h * 0.42f,
            Theme::col(Theme::bgDeep), 0.0f, h,
            true); // radial
        g.setGradientFill(vignette);
        g.fillRect(bounds);
    }

    // === Top bar ===
    auto topBar = bounds.removeFromTop(48);

    {
        juce::ColourGradient topGrad(
            Theme::col(Theme::bgRaised), 0.0f, static_cast<float>(topBar.getY()),
            Theme::col(Theme::bgBase), 0.0f, static_cast<float>(topBar.getBottom()),
            false);
        g.setGradientFill(topGrad);
        g.fillRect(topBar);
    }

    // Separator line under top bar
    g.setColour(Theme::col(Theme::line));
    g.drawHorizontalLine(topBar.getBottom() - 1, 0.0f, w);

    // Logo
    auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MAROLA___TTF,
        BinaryData::MAROLA___TTFSize);

    juce::Font logoFont(juce::FontOptions(typeface).withHeight(30.0f));
    g.setFont(logoFont);

    // Logo with a warm amber glow
    g.setColour(Theme::col(Theme::accent).withAlpha(0.35f));
    g.drawText("FisEQ", topBar.reduced(18, 0).translated(1, 1),
               juce::Justification::centredLeft);
    g.setColour(Theme::col(Theme::accentBright));
    g.drawText("FisEQ", topBar.reduced(18, 0),
               juce::Justification::centredLeft);

    // Subtitle — placed just right of the logo, away from the controls
    g.setFont(11.0f);
    g.setColour(Theme::col(Theme::textDim));
    auto subtitleArea = topBar.withTrimmedLeft(120).withTrimmedRight(230);
    g.drawText(viewMode == ViewMode::EQ ? "5-BAND PARAMETRIC EQ" : "SPECTROGRAM ANALYSER",
               subtitleArea, juce::Justification::centredLeft);

    // === Bottom area for frequency labels ===
    auto bottomBar = bounds.removeFromBottom(24);
    drawFrequencyLabels(g, bottomBar);

    // Bottom separator
    g.setColour(Theme::col(Theme::line));
    g.drawHorizontalLine(bottomBar.getY(), 0.0f, w);

    // === Left area for dB labels (EQ mode only) ===
    auto leftBar = bounds.removeFromLeft(36);
    if (viewMode == ViewMode::EQ)
        drawDBLabels(g, leftBar);

    // Left separator
    g.setColour(Theme::col(Theme::line));
    g.drawVerticalLine(leftBar.getRight(), static_cast<float>(leftBar.getY()),
                       static_cast<float>(leftBar.getBottom()));

    // === Graph area subtle inner shadow ===
    auto graphArea = bounds.reduced(1);
    {
        juce::ColourGradient topShadow(
            juce::Colour(0x22000000), 0.0f, static_cast<float>(graphArea.getY()),
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

    // View toggle button, left of the gain knob
    topBar.removeFromRight(8); // gap
    auto toggleArea = topBar.removeFromRight(120).reduced(0, 12);
    viewToggleButton.setBounds(toggleArea);

    auto graphArea = bounds.reduced(1);

    spectrumDisplay.setBounds(graphArea);
    eqCurveEditor.setBounds(graphArea);
    spectrogramDisplay.setBounds(graphArea);
}

void FisEQAudioProcessorEditor::drawFrequencyLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(11.0f);

    const float freqs[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    const char* labels[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };

    float graphLeft = static_cast<float>(area.getX() + 36);
    float graphWidth = static_cast<float>(area.getWidth() - 36);

    for (int i = 0; i < 10; ++i)
    {
        float normX = std::log10(freqs[i] / 20.0f) / std::log10(20000.0f / 20.0f);
        float x = graphLeft + normX * graphWidth;

        // Emphasize decade markers (brighter cream vs tobacco)
        bool isMajor = (i == 2 || i == 5 || i == 8); // 100, 1k, 10k
        g.setColour(Theme::col(isMajor ? Theme::textMid : Theme::textDim));

        g.drawText(labels[i],
                   static_cast<int>(x - 16), area.getY() + 4,
                   32, area.getHeight() - 4,
                   juce::Justification::centred);
    }
}

void FisEQAudioProcessorEditor::drawDBLabels(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setFont(11.0f);

    float h = static_cast<float>(area.getHeight());

    const float dbs[] = { 24, 12, 0, -12, -24 };

    for (float db : dbs)
    {
        float normY = 0.5f - (db / 24.0f) * 0.5f;
        int y = area.getY() + static_cast<int>(normY * h);

        bool isZero = (static_cast<int>(db) == 0);
        g.setColour(Theme::col(isZero ? Theme::textMid : Theme::textDim));

        juce::String label;
        if (db > 0)
            label = "+" + juce::String(static_cast<int>(db));
        else if (isZero)
            label = "0 dB";
        else
            label = juce::String(static_cast<int>(db));

        g.drawText(label,
                   area.getX() + 2, y - 7,
                   area.getWidth() - 4, 14,
                   juce::Justification::centredRight);
    }
}
