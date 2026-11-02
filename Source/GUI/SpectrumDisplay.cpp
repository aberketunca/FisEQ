#include "SpectrumDisplay.h"
#include "Theme.h"
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
    constexpr float minF = 20.0f;
    constexpr float maxF = 20000.0f;
    return std::log10(freq / minF) / std::log10(maxF / minF);
}

float SpectrumDisplay::xToFrequency(float normX) const
{
    constexpr float minF = 20.0f;
    constexpr float maxF = 20000.0f;
    return minF * std::pow(maxF / minF, normX);
}

float SpectrumDisplay::dbToY(float db, float h) const
{
    // Linear mapping: maxDBFS (0) at top, minDBFS (-90) at bottom
    float normalized = (db - minDBFS) / (maxDBFS - minDBFS);
    normalized = juce::jlimit(0.0f, 1.0f, normalized);
    return h * (1.0f - normalized);
}

void SpectrumDisplay::buildAnalyzerPath(juce::Path& path, float w, float h,
                                         const std::array<float, SpectrumAnalyzer::numBins>& mags) const
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

        // mags are already 0..1 normalized (0 = -100dB, 1 = 0dBFS)
        float mag = mags[static_cast<size_t>(bin0)] * (1.0f - frac)
                  + mags[static_cast<size_t>(bin1)] * frac;

        // Convert normalized back to dB for precise Y mapping
        float db = minDBFS + mag * (maxDBFS - minDBFS);
        float y = dbToY(db, h);

        path.lineTo(normX * w, y);
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

    // === Scientific grid ===

    // Frequency grid: octave markers + sub-octave
    {
        // Sub-octave lines (very faint)
        g.setColour(Theme::col(Theme::lineSoft).withAlpha(0.5f));
        const float subFreqs[] = {
            30, 40, 50, 60, 70, 80, 90,
            200, 300, 400, 500, 600, 700, 800, 900,
            2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000
        };
        for (float freq : subFreqs)
        {
            float x = frequencyToX(freq) * w;
            g.drawVerticalLine(static_cast<int>(x), 0.0f, h);
        }

        // Decade lines (brighter)
        g.setColour(Theme::col(Theme::line).withAlpha(0.8f));
        const float decadeFreqs[] = { 100, 1000, 10000 };
        for (float freq : decadeFreqs)
        {
            float x = frequencyToX(freq) * w;
            g.drawVerticalLine(static_cast<int>(x), 0.0f, h);
        }
    }

    // dBFS horizontal grid
    {
        // Every 10 dB (major)
        g.setColour(Theme::col(Theme::line).withAlpha(0.6f));
        for (float db = -80.0f; db <= -10.0f; db += 10.0f)
        {
            float y = dbToY(db, h);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
        }

        // Every 30 dB (emphasized)
        g.setColour(Theme::col(Theme::lineStrong).withAlpha(0.5f));
        for (float db : { -30.0f, -60.0f })
        {
            float y = dbToY(db, h);
            g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
        }
    }

    // === Peak hold spectrum (thin dotted appearance) ===
    {
        juce::Path peakPath;
        const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
        const int numBins = SpectrumAnalyzer::numBins;
        const int numPoints = static_cast<int>(w / 2.0f);

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

            float db = minDBFS + mag * (maxDBFS - minDBFS);
            float y = dbToY(db, h);

            if (i == 0)
                peakPath.startNewSubPath(normX * w, y);
            else
                peakPath.lineTo(normX * w, y);
        }

        // Subtle peak line
        g.setColour(Theme::col(Theme::textDim).withAlpha(0.4f));
        g.strokePath(peakPath, juce::PathStrokeType(0.75f));
    }

    // === Main spectrum fill (subtle, analytical) ===
    {
        juce::Path fillPath;
        buildAnalyzerPath(fillPath, w, h, displayMagnitudes);

        // Gradient: warm amber tint, very transparent — aged-instrument look
        juce::ColourGradient fillGrad(
            Theme::col(Theme::accentBright).withAlpha(0.16f), 0.0f, 0.0f,
            Theme::col(Theme::bgBase).withAlpha(0.02f), 0.0f, h,
            false);
        fillGrad.addColour(0.3, Theme::col(Theme::accent).withAlpha(0.13f));
        fillGrad.addColour(0.7, Theme::col(Theme::accentDeep).withAlpha(0.06f));

        g.setGradientFill(fillGrad);
        g.fillPath(fillPath);
    }

    // === Main spectrum line (crisp, analytical) ===
    {
        const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
        const int numBins = SpectrumAnalyzer::numBins;
        const int numPoints = static_cast<int>(w / 2.0f);

        juce::Path linePath;
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

            float db = minDBFS + mag * (maxDBFS - minDBFS);
            float y = dbToY(db, h);

            if (i == 0)
                linePath.startNewSubPath(normX * w, y);
            else
                linePath.lineTo(normX * w, y);
        }

        // Single clean stroke — no glow, just a precise line
        // Subtle outer for antialiasing
        g.setColour(Theme::col(Theme::accent).withAlpha(0.25f));
        g.strokePath(linePath, juce::PathStrokeType(2.5f));

        // Core line — warm amber, bright and precise
        g.setColour(Theme::col(Theme::spectrumLine).withAlpha(0.85f));
        g.strokePath(linePath, juce::PathStrokeType(1.0f));
    }
}

void SpectrumDisplay::resized()
{
}
