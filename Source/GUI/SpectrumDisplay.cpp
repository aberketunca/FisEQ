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
        // Smooth display values with asymmetric attack/decay
        for (size_t i = 0; i < displayMagnitudes.size(); ++i)
        {
            float target = rawMagnitudes[i];
            float current = displayMagnitudes[i];

            if (target > current)
                displayMagnitudes[i] = current + (target - current) * riseRate;
            else
                displayMagnitudes[i] = current * decayRate + target * (1.0f - decayRate);
        }
    }
    else
    {
        // No new data — just decay
        for (auto& m : displayMagnitudes)
            m *= decayRate;
    }

    repaint();
}

float SpectrumDisplay::frequencyToX(float freq) const
{
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;

    return (std::log10(freq / minFreq)) / (std::log10(maxFreq / minFreq));
}

float SpectrumDisplay::xToFrequency(float normX) const
{
    constexpr float minFreq = 20.0f;
    constexpr float maxFreq = 20000.0f;

    return minFreq * std::pow(maxFreq / minFreq, normX);
}

void SpectrumDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    if (w <= 0.0f || h <= 0.0f)
        return;

    // Build spectrum path
    spectrumPath.clear();
    spectrumPath.startNewSubPath(bounds.getX(), bounds.getBottom());

    const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
    const int numBins = SpectrumAnalyzer::numBins;

    // We'll draw one point per pixel (or per 2 pixels for efficiency)
    const int numPoints = static_cast<int>(w / 2.0f);

    for (int i = 0; i <= numPoints; ++i)
    {
        float normX = static_cast<float>(i) / static_cast<float>(numPoints);
        float freq = xToFrequency(normX);

        // Find which FFT bin this frequency corresponds to
        float binIndex = freq / binWidth;

        // Interpolate between bins
        int bin0 = static_cast<int>(binIndex);
        int bin1 = bin0 + 1;

        if (bin0 < 0) bin0 = 0;
        if (bin1 >= numBins) bin1 = numBins - 1;
        if (bin0 >= numBins) bin0 = numBins - 1;

        float frac = binIndex - static_cast<float>(bin0);
        frac = juce::jlimit(0.0f, 1.0f, frac);

        float mag = displayMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
                  + displayMagnitudes[static_cast<size_t>(bin1)] * frac;

        // Apply a slight power curve for visual appeal
        mag = std::pow(mag, 0.7f);

        float x = bounds.getX() + normX * w;
        float y = bounds.getBottom() - mag * h * 0.9f;

        spectrumPath.lineTo(x, y);
    }

    // Close the path
    spectrumPath.lineTo(bounds.getRight(), bounds.getBottom());
    spectrumPath.closeSubPath();

    // Fill with vertical gradient
    juce::ColourGradient gradient(
        topColour, bounds.getX(), bounds.getY(),
        bottomColour, bounds.getX(), bounds.getBottom(),
        false);
    gradient.addColour(0.5, midColour);

    g.setGradientFill(gradient);
    g.setOpacity(0.6f);
    g.fillPath(spectrumPath);

    // Draw the top line with a brighter stroke
    juce::Path strokePath;
    strokePath.startNewSubPath(bounds.getX(), bounds.getBottom());

    for (int i = 0; i <= numPoints; ++i)
    {
        float normX = static_cast<float>(i) / static_cast<float>(numPoints);
        float freq = xToFrequency(normX);

        float binIndex = freq / binWidth;
        int bin0 = static_cast<int>(binIndex);
        int bin1 = bin0 + 1;

        if (bin0 < 0) bin0 = 0;
        if (bin1 >= numBins) bin1 = numBins - 1;
        if (bin0 >= numBins) bin0 = numBins - 1;

        float frac = binIndex - static_cast<float>(bin0);
        frac = juce::jlimit(0.0f, 1.0f, frac);

        float mag = displayMagnitudes[static_cast<size_t>(bin0)] * (1.0f - frac)
                  + displayMagnitudes[static_cast<size_t>(bin1)] * frac;

        mag = std::pow(mag, 0.7f);

        float x = bounds.getX() + normX * w;
        float y = bounds.getBottom() - mag * h * 0.9f;

        if (i == 0)
            strokePath.startNewSubPath(x, y);
        else
            strokePath.lineTo(x, y);
    }

    g.setOpacity(1.0f);
    g.setColour(topColour.withAlpha(0.9f));
    g.strokePath(strokePath, juce::PathStrokeType(1.5f));

    // Draw frequency grid lines
    g.setColour(juce::Colours::white.withAlpha(0.06f));
    const float gridFreqs[] = { 50, 100, 200, 500, 1000, 2000, 5000, 10000 };

    for (float freq : gridFreqs)
    {
        float x = bounds.getX() + frequencyToX(freq) * w;
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }

    // Draw dB grid lines (horizontal)
    for (int i = 1; i <= 4; ++i)
    {
        float y = bounds.getBottom() - (static_cast<float>(i) / 5.0f) * h;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }
}

void SpectrumDisplay::resized()
{
    // Nothing special needed
}
