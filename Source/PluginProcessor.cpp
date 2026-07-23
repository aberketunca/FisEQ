#include "PluginProcessor.h"
#include "PluginEditor.h"

FisEQAudioProcessor::FisEQAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void FisEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
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

    // Pass audio through unchanged.
    // DSP will be added here later.
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

void FisEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}