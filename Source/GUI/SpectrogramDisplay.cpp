#include "SpectrogramDisplay.h"
#include "Theme.h"
#include <cmath>

SpectrogramDisplay::SpectrogramDisplay()
{
    setOpaque(false);
    buildColourLUT();
}

//==============================================================================
// Vintage 70s colour ramp.
//
// Inspired by faded Kodachrome / Ektachrome concert photography: deep inky
// plum shadows, bruised magenta, blood orange and amber midtones, climbing to
// a warm bleached cream highlight. Slightly desaturated and "dirty" rather
// than neon, so it reads as film rather than digital.
//==============================================================================
void SpectrogramDisplay::buildColourLUT()
{
    // Control stops along the ramp (position 0..1 -> colour).
    struct Stop { float pos; juce::uint8 r, g, b; };

    static const Stop stops[] = {
        { 0.00f,  14,  10,  18 },   // near-black plum (silence)
        { 0.14f,  38,  18,  40 },   // deep aubergine
        { 0.30f,  92,  28,  66 },   // bruised magenta
        { 0.46f, 158,  40,  54 },   // oxblood red
        { 0.62f, 208,  78,  38 },   // blood orange
        { 0.76f, 232, 132,  40 },   // amber / tobacco
        { 0.88f, 238, 182,  78 },   // faded gold
        { 1.00f, 244, 224, 176 }    // bleached cream highlight
    };
    constexpr int numStops = static_cast<int>(sizeof(stops) / sizeof(Stop));

    for (int i = 0; i < lutSize; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(lutSize - 1);

        // Find surrounding stops
        int s = 0;
        while (s < numStops - 2 && t > stops[s + 1].pos)
            ++s;

        const auto& a = stops[s];
        const auto& b = stops[s + 1];
        float span = juce::jmax(1.0e-4f, b.pos - a.pos);
        float f = juce::jlimit(0.0f, 1.0f, (t - a.pos) / span);

        auto lerp = [](juce::uint8 x, juce::uint8 y, float k) {
            return static_cast<juce::uint8>(juce::jlimit(0.0f, 255.0f,
                static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * k));
        };

        colourLUT[static_cast<size_t>(i)] = juce::Colour(
            lerp(a.r, b.r, f),
            lerp(a.g, b.g, f),
            lerp(a.b, b.b, f));
    }
}

juce::Colour SpectrogramDisplay::mapColour(float value) const
{
    value = juce::jlimit(0.0f, 1.0f, value);
    int idx = static_cast<int>(value * static_cast<float>(lutSize - 1));
    return colourLUT[static_cast<size_t>(idx)];
}

void SpectrogramDisplay::resized()
{
    int w = juce::jmax(1, getWidth());
    int h = juce::jmax(1, getHeight());

    // (Re)create the scrolling image at component resolution.
    juce::Image newImage(juce::Image::RGB, w, h, true);

    // Preserve old content by scaling it into the new image if we had one.
    if (imageReady && spectrogramImage.isValid())
    {
        juce::Graphics g(newImage);
        g.drawImage(spectrogramImage, 0, 0, w, h,
                    0, 0, spectrogramImage.getWidth(), spectrogramImage.getHeight());
    }
    else
    {
        // Fill with the "silence" colour so it doesn't start pure black.
        juce::Graphics g(newImage);
        g.fillAll(colourLUT[0]);
    }

    spectrogramImage = newImage;
    imageReady = true;
}

void SpectrogramDisplay::updateSpectrum(SpectrumAnalyzer& analyzer)
{
    if (analyzer.pullMagnitudes(frame))
    {
        pushColumn(frame);
        repaint();
    }
}

void SpectrogramDisplay::pushColumn(const std::array<float, SpectrumAnalyzer::numBins>& mags)
{
    if (!imageReady || !spectrogramImage.isValid())
        return;

    const int w = spectrogramImage.getWidth();
    const int h = spectrogramImage.getHeight();
    if (w < 2 || h < 2)   // need h >= 2 so (h - 1) divisor is safe
        return;

    const float binWidth = static_cast<float>(sampleRate) / static_cast<float>(SpectrumAnalyzer::fftSize);
    if (!(binWidth > 0.0f))   // guards sampleRate == 0 before the device is open
        return;

    // Scroll the existing image left by 1px.
    spectrogramImage.moveImageSection(0, 0, 1, 0, w - 1, h);

    // Draw the newest column on the right edge.
    juce::Image::BitmapData data(spectrogramImage, juce::Image::BitmapData::writeOnly);

    const int lastX = w - 1;
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);
    const int numBins = SpectrumAnalyzer::numBins;

    for (int y = 0; y < h; ++y)
    {
        // Map pixel row -> frequency (log scale, high freq at top).
        float norm = 1.0f - static_cast<float>(y) / static_cast<float>(h - 1);
        float logF = logMin + norm * (logMax - logMin);
        float freq = std::pow(10.0f, logF);

        float binIndex = freq / binWidth;
        int bin0 = juce::jlimit(0, numBins - 1, static_cast<int>(binIndex));
        int bin1 = juce::jlimit(0, numBins - 1, bin0 + 1);
        float frac = juce::jlimit(0.0f, 1.0f, binIndex - static_cast<float>(bin0));

        float mag = mags[static_cast<size_t>(bin0)] * (1.0f - frac)
                  + mags[static_cast<size_t>(bin1)] * frac;

        // Perceptual lift so quiet detail is visible (film-like response).
        mag = std::pow(juce::jlimit(0.0f, 1.0f, mag), 0.55f);

        auto c = mapColour(mag);
        data.setPixelColour(lastX, y, c);
    }
}

void SpectrogramDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;

    if (imageReady && spectrogramImage.isValid())
    {
        g.drawImageAt(spectrogramImage, 0, 0);
    }
    else
    {
        g.fillAll(colourLUT[0]);
    }

    // === Vintage finishing: vignette + scanline tint ===

    // Soft dark vignette at the corners for an aged-print feel.
    {
        auto fb = bounds.toFloat();
        juce::ColourGradient vignette(
            juce::Colours::transparentBlack, fb.getCentreX(), fb.getCentreY(),
            juce::Colour(0x66000000), fb.getX(), fb.getY(),
            true);
        g.setGradientFill(vignette);
        g.fillRect(bounds);
    }

    // Faint warm overlay to unify the palette (like aged film base).
    g.setColour(juce::Colour(0x120f0a06));
    g.fillRect(bounds);

    // === Frequency grid (subtle, warm) ===
    g.setColour(juce::Colour(0x22f4e0b0));
    const float gridFreqs[] = { 100, 1000, 10000 };
    float w = static_cast<float>(getWidth());
    float h = static_cast<float>(getHeight());
    const float logMin = std::log10(minFreq);
    const float logMax = std::log10(maxFreq);

    for (float freq : gridFreqs)
    {
        float norm = (std::log10(freq) - logMin) / (logMax - logMin);
        float y = h * (1.0f - norm);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
    }
}
