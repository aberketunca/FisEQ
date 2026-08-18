#include "EQCurveEditor.h"
#include <cmath>

EQCurveEditor::EQCurveEditor(FisEQAudioProcessor& p)
    : processor(p)
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::NormalCursor);
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

juce::String EQCurveEditor::formatFrequency(float freq)
{
    if (freq >= 1000.0f)
        return juce::String(freq / 1000.0f, 2) + " kHz";
    return juce::String(static_cast<int>(freq)) + " Hz";
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

    // Update tooltip values if showing
    if (showTooltip && tooltipBand >= 0)
    {
        auto prefix = "band" + juce::String(tooltipBand + 1) + "_";
        tooltipFreq = processor.parameters.getRawParameterValue(prefix + "freq")->load();
        tooltipGain = processor.parameters.getRawParameterValue(prefix + "gain")->load();
        tooltipQ    = processor.parameters.getRawParameterValue(prefix + "q")->load();
        tooltipPos  = nodes[static_cast<size_t>(tooltipBand)].position;
    }

    repaint();
}

void EQCurveEditor::drawBandwidthIndicator(juce::Graphics& g, int bandIndex, float w, float h)
{
    auto prefix = "band" + juce::String(bandIndex + 1) + "_";
    float freq = processor.parameters.getRawParameterValue(prefix + "freq")->load();
    float q    = processor.parameters.getRawParameterValue(prefix + "q")->load();
    float gain = processor.parameters.getRawParameterValue(prefix + "gain")->load();

    if (std::abs(gain) < 0.5f)
        return;

    auto colour = juce::Colour(bandColours[static_cast<size_t>(bandIndex)]);
    bool isActive = (bandIndex == hoveredBand || bandIndex == draggedBand);

    // Bandwidth in octaves: BW = freq/Q, approximate visual extent
    // -3dB points for a peak filter: f1 = f/sqrt(2^(1/Q)), f2 = f*sqrt(2^(1/Q))
    // Simpler approximation: half-gain bandwidth ≈ freq/Q
    float bwHz = freq / q;
    float f1 = freq - bwHz * 0.5f;
    float f2 = freq + bwHz * 0.5f;
    f1 = juce::jmax(f1, minFreq);
    f2 = juce::jmin(f2, maxFreq);

    float x1 = frequencyToX(f1) * w;
    float x2 = frequencyToX(f2) * w;

    float centerY = h * 0.5f;
    float nodeY = dbToY(gain);

    // Shaded bandwidth area
    juce::Path bwPath;
    bwPath.startNewSubPath(x1, centerY);
    bwPath.lineTo(x1, nodeY + (centerY - nodeY) * 0.7f);
    bwPath.quadraticTo(frequencyToX(freq) * w, nodeY,
                       x2, nodeY + (centerY - nodeY) * 0.7f);
    bwPath.lineTo(x2, centerY);
    bwPath.closeSubPath();

    g.setColour(colour.withAlpha(isActive ? 0.12f : 0.05f));
    g.fillPath(bwPath);
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

    juce::Path fillPath(bandPath);
    fillPath.lineTo(w, h * 0.5f);
    fillPath.lineTo(0.0f, h * 0.5f);
    fillPath.closeSubPath();

    bool isActive = (bandIndex == hoveredBand || bandIndex == draggedBand);
    g.setColour(colour.withAlpha(isActive ? 0.08f : 0.03f));
    g.fillPath(fillPath);

    g.setColour(colour.withAlpha(isActive ? 0.5f : 0.18f));
    g.strokePath(bandPath, juce::PathStrokeType(isActive ? 1.5f : 0.8f));
}

void EQCurveEditor::drawTooltip(juce::Graphics& g)
{
    if (!showTooltip || tooltipBand < 0)
        return;

    // Format strings
    auto freqStr = formatFrequency(tooltipFreq);
    auto gainStr = (tooltipGain >= 0.0f ? "+" : "") + juce::String(tooltipGain, 1) + " dB";
    auto qStr    = "Q " + juce::String(tooltipQ, 2);

    // Tooltip box
    float boxW = 95.0f;
    float boxH = 48.0f;

    // Position above node, offset if near edges
    float tx = tooltipPos.x - boxW * 0.5f;
    float ty = tooltipPos.y - boxH - 14.0f;

    // Clamp to bounds
    tx = juce::jlimit(4.0f, static_cast<float>(getWidth()) - boxW - 4.0f, tx);
    ty = juce::jlimit(4.0f, static_cast<float>(getHeight()) - boxH - 4.0f, ty);

    auto boxRect = juce::Rectangle<float>(tx, ty, boxW, boxH);

    // Background
    g.setColour(juce::Colour(0xee0f172a));
    g.fillRoundedRectangle(boxRect, 4.0f);

    // Border
    auto colour = juce::Colour(bandColours[static_cast<size_t>(tooltipBand)]);
    g.setColour(colour.withAlpha(0.6f));
    g.drawRoundedRectangle(boxRect, 4.0f, 1.0f);

    // Text
    g.setFont(11.0f);
    g.setColour(juce::Colour(0xffe2e8f0));
    g.drawText(freqStr, boxRect.reduced(6, 4).removeFromTop(14), juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffa5f3fc));
    auto midLine = boxRect.reduced(6, 0).withTrimmedTop(16).removeFromTop(14);
    g.drawText(gainStr, midLine.removeFromLeft(midLine.getWidth() * 0.55f), juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff94a3b8));
    g.drawText(qStr, midLine, juce::Justification::centredRight);
}

void EQCurveEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    if (w <= 0 || h <= 0)
        return;

    // === Grid ===
    float centerY = h * 0.5f;

    // 0 dB center line
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawHorizontalLine(static_cast<int>(centerY), 0.0f, w);

    // +/-6, 12, 18 dB lines
    g.setColour(juce::Colours::white.withAlpha(0.035f));
    for (float db : { -18.0f, -12.0f, -6.0f, 6.0f, 12.0f, 18.0f })
    {
        float y = dbToY(db);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, w);
    }

    // === Bandwidth indicators (behind curves) ===
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        if (i == hoveredBand || i == draggedBand)
            drawBandwidthIndicator(g, i, w, h);
    }

    // === Per-band curves ===
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

    // Composite fill
    juce::Path fillPath(curvePath);
    fillPath.lineTo(w, centerY);
    fillPath.lineTo(0.0f, centerY);
    fillPath.closeSubPath();

    g.setColour(juce::Colours::white.withAlpha(0.03f));
    g.fillPath(fillPath);

    // 3-pass glow stroke
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.strokePath(curvePath, juce::PathStrokeType(4.5f));

    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(curvePath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.strokePath(curvePath, juce::PathStrokeType(1.0f));

    // === Band nodes ===
    for (int i = 0; i < FisEQAudioProcessor::numBands; ++i)
    {
        auto& node = nodes[static_cast<size_t>(i)];
        bool isActive = (i == hoveredBand || i == draggedBand);
        float r = isActive ? nodeRadius * 1.4f : nodeRadius;

        // Connection line to 0 dB
        g.setColour(node.colour.withAlpha(isActive ? 0.3f : 0.1f));
        g.drawLine(node.position.x, node.position.y,
                   node.position.x, centerY, 1.0f);

        // Radial glow
        {
            float glowR = r * 2.8f;
            juce::ColourGradient glow(
                node.colour.withAlpha(isActive ? 0.3f : 0.12f),
                node.position.x, node.position.y,
                node.colour.withAlpha(0.0f),
                node.position.x + glowR, node.position.y,
                true);
            g.setGradientFill(glow);
            g.fillEllipse(node.position.x - glowR,
                          node.position.y - glowR,
                          glowR * 2.0f, glowR * 2.0f);
        }

        // Main circle
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

        // Ring
        g.setColour(juce::Colours::white.withAlpha(isActive ? 0.7f : 0.3f));
        g.drawEllipse(node.position.x - r, node.position.y - r,
                      r * 2.0f, r * 2.0f,
                      isActive ? 1.6f : 0.9f);

        // Specular highlight
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        float dotR = r * 0.22f;
        g.fillEllipse(node.position.x - dotR - r * 0.2f,
                      node.position.y - dotR - r * 0.25f,
                      dotR * 2.0f, dotR * 2.0f);
    }

    // === Tooltip (drawn last, on top) ===
    drawTooltip(g);
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

    if (draggedBand >= 0)
    {
        showTooltip = true;
        tooltipBand = draggedBand;
    }
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
    showTooltip = false;
    tooltipBand = -1;
    repaint();
}

void EQCurveEditor::mouseMove(const juce::MouseEvent& e)
{
    int newHover = findBandAt(e.position);
    if (newHover != hoveredBand)
    {
        hoveredBand = newHover;

        if (hoveredBand >= 0)
        {
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            showTooltip = true;
            tooltipBand = hoveredBand;
        }
        else
        {
            setMouseCursor(juce::MouseCursor::NormalCursor);
            showTooltip = false;
            tooltipBand = -1;
        }

        repaint();
    }
}

void EQCurveEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
    int band = findBandAt(e.position);
    if (band < 0)
        return;

    // Reset gain to 0 dB (Pro-Q convention)
    auto prefix = "band" + juce::String(band + 1) + "_";

    if (auto* param = processor.parameters.getParameter(prefix + "gain"))
        param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(0.0f));
}

void EQCurveEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    int band = findBandAt(e.position);
    if (band < 0)
    {
        Component::mouseWheelMove(e, wheel);
        return;
    }

    // Scroll wheel adjusts Q
    auto prefix = "band" + juce::String(band + 1) + "_";
    auto* param = processor.parameters.getParameter(prefix + "q");
    if (param == nullptr)
        return;

    float currentQ = processor.parameters.getRawParameterValue(prefix + "q")->load();

    // Logarithmic Q adjustment for natural feel
    float multiplier = (wheel.deltaY > 0) ? 1.15f : (1.0f / 1.15f);
    float newQ = juce::jlimit(0.1f, 10.0f, currentQ * multiplier);

    param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(newQ));

    // Show tooltip with updated Q
    showTooltip = true;
    tooltipBand = band;
    repaint();
}
