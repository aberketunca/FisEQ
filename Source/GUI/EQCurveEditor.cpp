#include "EQCurveEditor.h"
#include <cmath>

EQCurveEditor::EQCurveEditor(FisEQAudioProcessor& p)
    : processor(p)
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

float EQCurveEditor::frequencyToX(float freq) const
{
    return std::log10(freq / minFreq) / std::log10(maxFreq / minFreq);
}

float EQCurveEditor::xToFrequency(float normX) const
{
    return minFreq * std::pow(maxFreq / minFreq, normX);
}

float EQCurveEditor::dbToY(float db) const
{
    float h = static_cast<float>(getHeight());
    return h * 0.5f - (db / maxDB) * h * 0.5f;
}

float EQCurveEditor::yToDb(float y) const
{
    float h = static_cast<float>(getHeight());
    return -(y - h * 0.5f) / (h * 0.5f) * maxDB;
}

void EQCurveEditor::updateCurve()
{
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        auto prefix = "band" + juce::String(i + 1) + "_";
        float freq = processor.parameters.getRawParameterValue(prefix + "freq")->load();
        float gain = processor.parameters.getRawParameterValue(prefix + "gain")->load();

        float x = frequencyToX(freq) * static_cast<float>(getWidth());
        float y = dbToY(gain);

        nodes[static_cast<size_t>(i)].position = { x, y };
        nodes[static_cast<size_t>(i)].colour = juce::Colour(bandColours[static_cast<size_t>(i)]);
    }

    repaint();
}

void EQCurveEditor::drawPerBandCurve(juce::Graphics& g, int bandIndex, float w, float h)
{
    auto& band = processor.getBand(bandIndex);
    auto colour = juce::Colour(bandColours[static_cast<size_t>(bandIndex)]);

    juce::Path bandPath;
    const int numPoints = static_cast<int>(w / 2.0f);

    for (int px = 0; px <= numPoints; ++px)
    {
        float normX = static_cast<float>(px) / static_cast<float>(numPoints);
        float freq = xToFrequency(normX);

        double mag = band.getMagnitudeAtFrequency(static_cast<double>(freq));
        float db = static_cast<float>(juce::Decibels::gainToDecibels(mag, -100.0));
        float y = dbToY(db);
        float x = normX * w;

        if (px == 0)
            bandPath.startNewSubPath(x, y);
        else
            bandPath.lineTo(x, y);
    }

    // Subtle per-band fill
    juce::Path fillPath(bandPath);
    fillPath.lineTo(w, h * 0.5f);
    fillPath.lineTo(0.0f, h * 0.5f);
    fillPath.closeSubPath();

    bool isActive = (bandIndex == hoveredBand || bandIndex == draggedBand);
    g.setColour(colour.withAlpha(isActive ? 0.08f : 0.03f));
    g.fillPath(fillPath);

    // Per-band curve stroke (subtle)
    g.setColour(colour.withAlpha(isActive ? 0.45f : 0.15f));
    g.strokePath(bandPath, juce::PathStrokeType(isActive ? 1.5f : 0.8f));
}

void EQCurveEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    if (w <= 0 || h <= 0)
        return;

    // === Grid: 0 dB center line (most prominent) ===
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    float centerY = h * 0.5f;
    g.drawHorizontalLine(static_cast<int>(centerY), 0.0f, w);

    // +/-6, 12, 18 dB lines
    g.setColour(juce::Colours::white.withAlpha(0.035f));
    for (float db : { -18.0f, -12.0f, -6.0f, 6.0f, 12.0f, 18.0f })
    {
        float y = dbToY(db);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
    }

    // === Per-band individual curves (background layer) ===
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
        drawPerBandCurve(g, i, w, h);

    // === Composite EQ curve ===
    juce::Path curvePath;
    const int numPoints = static_cast<int>(w);

    for (int px = 0; px <= numPoints; ++px)
    {
        float normX = static_cast<float>(px) / w;
        float freq = xToFrequency(normX);

        double mag = processor.getCompositeMagnitude(static_cast<double>(freq));
        float db = static_cast<float>(juce::Decibels::gainToDecibels(mag, -100.0));
        float y = dbToY(db);

        if (px == 0)
            curvePath.startNewSubPath(static_cast<float>(px), y);
        else
            curvePath.lineTo(static_cast<float>(px), y);
    }

    // Composite fill (subtle white sheen above/below 0 dB)
    juce::Path fillPath(curvePath);
    fillPath.lineTo(w, centerY);
    fillPath.lineTo(0.0f, centerY);
    fillPath.closeSubPath();

    g.setColour(juce::Colours::white.withAlpha(0.03f));
    g.fillPath(fillPath);

    // Composite curve: 3-pass glow
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.strokePath(curvePath, juce::PathStrokeType(5.0f));

    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(curvePath, juce::PathStrokeType(2.5f));

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.strokePath(curvePath, juce::PathStrokeType(1.2f));

    // === Band nodes ===
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        auto& node = nodes[static_cast<size_t>(i)];
        bool isActive = (i == hoveredBand || i == draggedBand);
        float r = isActive ? nodeRadius * 1.5f : nodeRadius;

        // Connection line from node to 0 dB line
        g.setColour(node.colour.withAlpha(isActive ? 0.3f : 0.12f));
        g.drawLine(node.position.x, node.position.y,
                   node.position.x, centerY, 1.0f);

        // Outer radial glow
        {
            float glowR = r * 3.0f;
            juce::ColourGradient glow(
                node.colour.withAlpha(isActive ? 0.35f : 0.15f),
                node.position.x, node.position.y,
                node.colour.withAlpha(0.0f),
                node.position.x + glowR, node.position.y,
                true); // radial
            g.setGradientFill(glow);
            g.fillEllipse(node.position.x - glowR,
                          node.position.y - glowR,
                          glowR * 2.0f, glowR * 2.0f);
        }

        // Main circle — filled with slight gradient
        {
            float d = r * 2.0f;
            juce::ColourGradient nodeFill(
                node.colour.brighter(0.3f),
                node.position.x, node.position.y - r,
                node.colour.darker(0.2f),
                node.position.x, node.position.y + r,
                false);
            g.setGradientFill(nodeFill);
            g.fillEllipse(node.position.x - r, node.position.y - r, d, d);
        }

        // Highlight ring
        g.setColour(juce::Colours::white.withAlpha(isActive ? 0.7f : 0.35f));
        g.drawEllipse(node.position.x - r, node.position.y - r,
                      r * 2.0f, r * 2.0f,
                      isActive ? 1.8f : 1.0f);

        // Inner highlight dot
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        float dotR = r * 0.25f;
        g.fillEllipse(node.position.x - dotR - r * 0.2f,
                      node.position.y - dotR - r * 0.25f,
                      dotR * 2.0f, dotR * 2.0f);
    }
}

void EQCurveEditor::resized()
{
}

int EQCurveEditor::findBandAt(juce::Point<float> pos) const
{
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        if (nodes[static_cast<size_t>(i)].position.getDistanceFrom(pos) < nodeHitRadius)
            return i;
    }
    return -1;
}

void EQCurveEditor::mouseDown(const juce::MouseEvent& e)
{
    draggedBand = findBandAt(e.position);
}

void EQCurveEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedBand < 0)
        return;

    float w = static_cast<float>(getWidth());
    float normX = juce::jlimit(0.0f, 1.0f, e.position.x / w);
    float freq = xToFrequency(normX);
    float db = yToDb(e.position.y);

    freq = juce::jlimit(minFreq, maxFreq, freq);
    db = juce::jlimit(minDB, maxDB, db);

    auto prefix = "band" + juce::String(draggedBand + 1) + "_";

    if (auto* param = processor.parameters.getParameter(prefix + "freq"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(freq));

    if (auto* param = processor.parameters.getParameter(prefix + "gain"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(db));
}

void EQCurveEditor::mouseUp(const juce::MouseEvent&)
{
    draggedBand = -1;
}

void EQCurveEditor::mouseMove(const juce::MouseEvent& e)
{
    int newHover = findBandAt(e.position);
    if (newHover != hoveredBand)
    {
        hoveredBand = newHover;

        if (hoveredBand >= 0)
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        else
            setMouseCursor(juce::MouseCursor::CrosshairCursor);

        repaint();
    }
}
