#include "PluginProcessor.h"
#include "PluginEditor.h"

FisEQAudioProcessor::FisEQAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this,
                 nullptr,
                 "PARAMETERS",
                 createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
FisEQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "frequency",
        "Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
        1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "q",
        "Q",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f),
        0.707f));

    return { params.begin(), params.end() };
}

void FisEQAudioProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock)
{
    eqBand.prepare(sampleRate,
                   samplesPerBlock,
                   getTotalNumOutputChannels());
}

void FisEQAudioProcessor::releaseResources()
{
}

bool FisEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void FisEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    float frequency =
        parameters.getRawParameterValue("frequency")->load();

    float gain =
        parameters.getRawParameterValue("gain")->load();

    float q =
        parameters.getRawParameterValue("q")->load();

    eqBand.update(frequency, gain, q);
    eqBand.process(buffer);
}

juce::AudioProcessorEditor* FisEQAudioProcessor::createEditor()
{
    return new FisEQAudioProcessorEditor(*this);
}

bool FisEQAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String FisEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void FisEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void FisEQAudioProcessor::setStateInformation(const void* data,
                                              int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FisEQAudioProcessor();
}