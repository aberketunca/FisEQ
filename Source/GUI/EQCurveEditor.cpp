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
    // 0 dB is at centre; +maxDB is at top; -maxDB is at bottom
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
    // Update node positions from current parameter values
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

void EQCurveEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    if (w <= 0 || h <= 0)
        return;

    // Draw 0 dB center line
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawHorizontalLine(static_cast<int>(h * 0.5f), 0.0f, w);

    // Draw ±6, ±12, ±18 dB lines
    for (float db : { -18.0f, -12.0f, -6.0f, 6.0f, 12.0f, 18.0f })
    {
        float y = dbToY(db);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
    }

    // Draw composite EQ curve
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

    // Stroke the curve
    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.strokePath(curvePath, juce::PathStrokeType(2.0f));

    // Fill below/above 0 dB with subtle colour
    juce::Path fillPath(curvePath);
    fillPath.lineTo(w, h * 0.5f);
    fillPath.lineTo(0.0f, h * 0.5f);
    fillPath.closeSubPath();

    g.setColour(juce::Colours::white.withAlpha(0.04f));
    g.fillPath(fillPath);

    // Draw band nodes
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        auto& node = nodes[static_cast<size_t>(i)];
        float r = (i == hoveredBand || i == draggedBand) ? nodeRadius * 1.4f : nodeRadius;

        // Outer glow
        g.setColour(node.colour.withAlpha(0.25f));
        g.fillEllipse(node.position.x - r * 1.5f,
                      node.position.y - r * 1.5f,
                      r * 3.0f, r * 3.0f);

        // Solid circle
        g.setColour(node.colour);
        g.fillEllipse(node.position.x - r,
                      node.position.y - r,
                      r * 2.0f, r * 2.0f);

        // White border
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawEllipse(node.position.x - r,
                      node.position.y - r,
                      r * 2.0f, r * 2.0f, 1.2f);

        // Band number label
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText(juce::String(i + 1),
                   juce::Rectangle<float>(node.position.x - 6, node.position.y - 5, 12, 10),
                   juce::Justification::centred);
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
