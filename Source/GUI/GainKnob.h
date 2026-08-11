#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom-drawn gain knob with a modern look.
 * Arc-style indicator with glow, value label, and title.
 */
class GainKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GainKnobLookAndFeel() = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        float centreX = bounds.getCentreX();
        float centreY = bounds.getCentreY();

        float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // === Background track (dark ring) ===
        juce::Path bgArc;
        bgArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f,
                            0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1e293b));
        g.strokePath(bgArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // === Active arc (cyan glow) ===
        if (std::abs(sliderPosProportional - 0.5f) > 0.01f)
        {
            float startArc = rotaryStartAngle + 0.5f * (rotaryEndAngle - rotaryStartAngle);
            float endArc = angle;

            // Outer glow
            juce::Path glowArc;
            glowArc.addCentredArc(centreX, centreY, radius - 4.0f, radius - 4.0f,
                                  0.0f, startArc, endArc, true);
            g.setColour(juce::Colour(0xff06b6d4).withAlpha(0.25f));
            g.strokePath(glowArc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));

            // Core arc
            g.setColour(juce::Colour(0xff22d3ee));
            g.strokePath(glowArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        // === Center dot / pointer ===
        float pointerLength = radius - 10.0f;
        float pointerX = centreX + pointerLength * std::cos(angle - juce::MathConstants<float>::halfPi);
        float pointerY = centreY + pointerLength * std::sin(angle - juce::MathConstants<float>::halfPi);

        // Pointer dot with glow
        g.setColour(juce::Colour(0xff22d3ee).withAlpha(0.4f));
        g.fillEllipse(pointerX - 4.5f, pointerY - 4.5f, 9.0f, 9.0f);
        g.setColour(juce::Colour(0xffa5f3fc));
        g.fillEllipse(pointerX - 2.5f, pointerY - 2.5f, 5.0f, 5.0f);

        // === Center circle ===
        float innerR = radius * 0.38f;
        juce::ColourGradient centerGrad(
            juce::Colour(0xff1e293b), centreX, centreY - innerR,
            juce::Colour(0xff0f172a), centreX, centreY + innerR,
            false);
        g.setGradientFill(centerGrad);
        g.fillEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

        g.setColour(juce::Colour(0xff334155));
        g.drawEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 0.8f);

        // === Value text inside knob ===
        float val = static_cast<float>(slider.getValue());
        juce::String valText;
        if (val > 0.05f)
            valText = "+" + juce::String(val, 1);
        else if (val < -0.05f)
            valText = juce::String(val, 1);
        else
            valText = "0.0";

        g.setColour(juce::Colour(0xffe2e8f0));
        g.setFont(10.0f);
        g.drawText(valText,
                   static_cast<int>(centreX - 18), static_cast<int>(centreY - 6), 36, 12,
                   juce::Justification::centred);
    }
};

class GainKnob : public juce::Component
{
public:
    GainKnob()
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setLookAndFeel(&lnf);
        slider.setPopupDisplayEnabled(false, false, this);
        addAndMakeVisible(slider);
    }

    ~GainKnob() override
    {
        slider.setLookAndFeel(nullptr);
    }

    juce::Slider& getSlider() { return slider; }

    void paint(juce::Graphics& g) override
    {
        // Label below knob
        g.setFont(9.0f);
        g.setColour(juce::Colour(0xff64748b));
        g.drawText("GAIN", getLocalBounds().removeFromBottom(14),
                   juce::Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromBottom(14); // label space
        slider.setBounds(bounds);
    }

private:
    juce::Slider slider;
    GainKnobLookAndFeel lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnob)
};
