#include "SpectrumDisplay.h"
#include <cmath>

SpectrumDisplay::SpectrumDisplay()
{
    setOpaque(false);
}

void SpectrumDisplay::updateSpectrum(SpectrumAnalyzer& analyzer)
{
    if (analyzer.pullMagnitudes(rawMagnitudes))
    {
        for (size_t i = 0; i < displayMagnitudes.size(); ++i)
        {
            float target = rawMagnitudes[i];
            float current = displayMagnitudes[i];

            if (target > current)
                displayMagnitudes[i] = current + (target - current) * riseRate;
            else
                displayMagnitudes[i] = current * decayRate + target * (1.0f - decayRate);

            // Peak hold
            if (displayMagnitudes[i] > peakMagnitudes[i])
                peakMagnitudes[i] = displayMagnitudes[i];
            else
                peakMagnitudes[i] *= peakDecay;
        }
    }
    else
    {
        for (size_t i = 0; i < displayMagnitudes.size(); ++i)
        {
            displayMagnitudes[i] *= decayRate;
            peakMagnitudes[i] *= peakDecay;
        }
    }

    repaint();
}

float SpectrumDisplay::frequencyToX(float freq) const
{
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    return std::log10(freq / minFreq) / std::log10(maxFreq / minFreq);
}

float SpectrumDisplay::xToFrequency(float normX) const
{
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;
    return minFreq * std::pow(maxFreq / minFreq, normX);
}

void SpectrumDisplay::buildSpectrumPath(juce::Path& path, float w, float h, float heightScale) const
{
    const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
    const int numBins = SpectrumAnalyzer::numBins;
    const int numPoints = static_cast<int>(w / 2.0f);

    path.clear();
    path.startNewSubPath(0.0f, h);

    for (int i = 0; i <= numPoints; ++i)
    {
        float normX = static_cast<float>(i) / static_cast<float>(numPoints);
        float freq = xToFrequency(normX);
        float binIndex = freq / binWidth;

        int bin0 = juce::jlimit(0, numBins - 1, static_cast<int>(binIndex));
        int bin1 = juce::jlimit(0, numBins - 1, bin0 + 1);

        float frac = juce::jlimit(0.0f, 1.0f, binIndex - static_cast<float>(bin0));

        float mag = displayMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
                  + displayMagnitudes[static_cast<size_t>(bin1)] * frac;

        mag = std::pow(mag, 0.65f);

        float x = normX * w;
        float y = h - mag * h * heightScale;

        path.lineTo(x, y);
    }

    path.lineTo(w, h);
    path.closeSubPath();
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    if (w <= 0.0f || h <= 0.0f)
        return;

    // === Layer 1: Deep background fill (subtle) ===
    {
        juce::Path bgPath;
        const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
        const int numBins = SpectrumAnalyzer::numBins;
        const int numPoints = static_cast<int>(w / 2.0f);

        bgPath.startNewSubPath(0.0f, h);
        for (int i = 0; i <= numPoints; ++i)
        {
            float normX = static_cast<float>(i) / static_cast<float>(numPoints);
            float freq = xToFrequency(normX);
            float binIndex = freq / binWidth;

            int bin0 = juce::jlimit(0, numBins - 1, static_cast<int>(binIndex));
            int bin1 = juce::jlimit(0, numBins - 1, bin0 + 1);
            float frac = juce::jlimit(0.0f, 1.0f, binIndex - static_cast<float>(bin0));

            float mag = peakMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
                      + peakMagnitudes[static_cast<size_t>(bin1)] * frac;
            mag = std::pow(mag, 0.65f);

            bgPath.lineTo(normX * w, h - mag * h * 0.88f);
        }
        bgPath.lineTo(w, h);
        bgPath.closeSubPath();

        juce::ColourGradient peakGrad(
            juce::Colour(0x18845ec2), 0.0f, 0.0f,
            juce::Colour(0x05200a3d), 0.0f, h,
            false);
        g.setGradientFill(peakGrad);
        g.fillPath(bgPath);
    }

    // === Layer 2: Main spectrum fill ===
    {
        juce::Path mainPath;
        buildSpectrumPath(mainPath, w, h, 0.88f);

        // Rich gradient: teal-blue at bottom fading to violet at top
        juce::ColourGradient mainGrad(
            juce::Colour(0xff7c3aed), 0.0f, 0.0f,      // violet top
            juce::Colour(0xff0d1b2a), 0.0f, h,          // near-black bottom
            false);
        mainGrad.addColour(0.25, juce::Colour(0xff6366f1));  // indigo
        mainGrad.addColour(0.5,  juce::Colour(0xff06b6d4));  // cyan
        mainGrad.addColour(0.75, juce::Colour(0xff0e7490));  // dark cyan

        g.setGradientFill(mainGrad);
        g.setOpacity(0.55f);
        g.fillPath(mainPath);
    }

    // === Layer 3: Brighter inner fill (narrower) ===
    {
        juce::Path innerPath;
        buildSpectrumPath(innerPath, w, h, 0.85f);

        juce::ColourGradient innerGrad(
            juce::Colour(0xff22d3ee), 0.0f, h * 0.2f,
            juce::Colour(0xff0f172a), 0.0f, h,
            false);
        innerGrad.addColour(0.4, juce::Colour(0xff0ea5e9));

        g.setGradientFill(innerGrad);
        g.setOpacity(0.3f);
        g.fillPath(innerPath);
    }

    // === Layer 4: Top edge glow stroke ===
    {
        const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
        const int numBins = SpectrumAnalyzer::numBins;
        const int numPoints = static_cast<int>(w / 2.0f);

        juce::Path strokePath;
        for (int i = 0; i <= numPoints; ++i)
        {
            float normX = static_cast<float>(i) / static_cast<float>(numPoints);
            float freq = xToFrequency(normX);
            float binIndex = freq / binWidth;

            int bin0 = juce::jlimit(0, numBins - 1, static_cast<int>(binIndex));
            int bin1 = juce::jlimit(0, numBins - 1, bin0 + 1);
            float frac = juce::jlimit(0.0f, 1.0f, binIndex - static_cast<float>(bin0));

            float mag = displayMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
                      + displayMagnitudes[static_cast<size_t>(bin1)] * frac;
            mag = std::pow(mag, 0.65f);

            float x = normX * w;
            float y = h - mag * h * 0.88f;

            if (i == 0)
                strokePath.startNewSubPath(x, y);
            else
                strokePath.lineTo(x, y);
        }

        // Outer glow (thick, low alpha)
        g.setOpacity(1.0f);
        g.setColour(juce::Colour(0xff22d3ee).withAlpha(0.2f));
        g.strokePath(strokePath, juce::PathStrokeType(4.0f));

        // Inner glow
        g.setColour(juce::Colour(0xff67e8f9).withAlpha(0.4f));
        g.strokePath(strokePath, juce::PathStrokeType(2.0f));

        // Core line
        g.setColour(juce::Colour(0xffa5f3fc).withAlpha(0.85f));
        g.strokePath(strokePath, juce::PathStrokeType(1.0f));
    }

    // === Grid lines (very subtle) ===
    g.setColour(juce::Colours::white.withAlpha(0.04f));
    const float gridFreqs[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000 };
    for (float freq : gridFreqs)
    {
        float x = frequencyToX(freq) * w;
        g.drawVerticalLine(static_cast<int>(x), 0.0f, h);
    }

    for (int i = 1; i <= 4; ++i)
    {
        float y = h - (static_cast<float>(i) / 5.0f) * h;
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
    }
}

void SpectrumDisplay::resized()
{
}
