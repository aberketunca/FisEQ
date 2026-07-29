#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <BinaryData.h>

FisEQAudioProcessorEditor::FisEQAudioProcessorEditor(FisEQAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(600, 400);

    auto setupSlider = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(
            juce::Slider::TextBoxBelow,
            false,
            70,
            20);
    };

    setupSlider(frequencySlider);
    setupSlider(gainSlider);
    setupSlider(qSlider);

    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(qSlider);

    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    gainLabel.setText("Gain", juce::dontSendNotification);
    qLabel.setText("Q", juce::dontSendNotification);

    frequencyLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setJustificationType(juce::Justification::centred);
    qLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(frequencyLabel);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(qLabel);

    frequencyAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters,
        "frequency",
        frequencySlider);

    gainAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters,
        "gain",
        gainSlider);

    qAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.parameters,
        "q",
        qSlider);
}

void FisEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
        juce::Rectangle<int> graphArea(
    40,
    60,
    getWidth() - 80,
    110
    );
    g.setColour(juce::Colour(0xff1b1b1b));
    g.fillRoundedRectangle(
    graphArea.toFloat(),
    8.0f
    );
    g.setColour(juce::Colours::grey);

    g.drawRoundedRectangle(
        graphArea.toFloat(),
        8.0f,
        1.0f
    );
    g.setColour(juce::Colours::darkgrey);
    
    auto frequency =
    audioProcessor.parameters.getRawParameterValue("frequency")->load();

    auto gain =
        audioProcessor.parameters.getRawParameterValue("gain")->load();

    auto q =
        audioProcessor.parameters.getRawParameterValue("q")->load();
    juce::Path response;


    for (int x = 0; x < graphArea.getWidth(); ++x)
    {
        float normX = (float)x / (float)graphArea.getWidth();

        float freqPos =
            std::log10(frequency / 20.0f)
            / std::log10(20000.0f / 20.0f);

        float sigma = 0.15f / q;

        float bell =
            std::exp(
                -0.5f *
                std::pow((normX - freqPos) / sigma, 2.0f)
            );

        float y =
            graphArea.getCentreY()
            - gain * bell * 3.0f;

        if (x == 0)
            response.startNewSubPath(
                (float)graphArea.getX(),
                y);
        else
            response.lineTo(
                (float)(graphArea.getX() + x),
                y);
    }
    g.setColour(juce::Colour(0xff1640C9));
    g.strokePath(
        response,
        juce::PathStrokeType(2.5f)
    );

    g.drawHorizontalLine(
        graphArea.getCentreY(),
        (float)graphArea.getX(),
        (float)graphArea.getRight()
    );

    g.setColour(juce::Colour(0xff1640C9));

    auto typeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MAROLA___TTF,
        BinaryData::MAROLA___TTFSize);

    juce::Font logoFont(typeface);
    logoFont.setHeight(40.0f);

    g.setFont(logoFont);

    g.drawFittedText(
        "FisEQ",
        0,
        10,
        getWidth(),
        40,
        juce::Justification::centred,
        1
    );
}

void FisEQAudioProcessorEditor::resized()
{
    frequencySlider.setBounds(60, 120, 120, 120);
    gainSlider.setBounds(240, 120, 120, 120);
    qSlider.setBounds(420, 120, 120, 120);

    frequencyLabel.setBounds(60, 245, 120, 20);
    gainLabel.setBounds(240, 245, 120, 20);
    qLabel.setBounds(420, 245, 120, 20);
}