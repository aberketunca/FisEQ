#include "PluginProcessor.h"
#include "PluginEditor.h"

FisEQAudioProcessor::FisEQAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
FisEQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    const std::array<juce::String, numBands> bandNames = {
        "Low Shelf", "Low Mid", "Mid", "High Mid", "High Shelf"
    };

    const std::array<float, numBands> defaults = {
        80.0f, 300.0f, 1000.0f, 4000.0f, 12000.0f
    };

    for (int i = 0; i < numBands; ++i)
    {
        auto prefix = "band" + juce::String(i + 1) + "_";

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "freq",
            bandNames[static_cast<size_t>(i)] + " Freq",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
            defaults[static_cast<size_t>(i)]));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "gain",
            bandNames[static_cast<size_t>(i)] + " Gain",
            juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
            0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "q",
            bandNames[static_cast<size_t>(i)] + " Q",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f),
            0.707f));
    }

    return { params.begin(), params.end() };
}

void FisEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for (auto& band : bands)
        band.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    analyzer.reset();
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

    // Update and process each band
    for (int i = 0; i < numBands; ++i)
    {
        auto prefix = "band" + juce::String(i + 1) + "_";

        float freq = parameters.getRawParameterValue(prefix + "freq")->load();
        float gain = parameters.getRawParameterValue(prefix + "gain")->load();
        float q    = parameters.getRawParameterValue(prefix + "q")->load();

        bands[static_cast<size_t>(i)].update(freq, gain, q, bandTypes[static_cast<size_t>(i)]);
        bands[static_cast<size_t>(i)].process(buffer);
    }

    // Push post-EQ audio to spectrum analyzer (mono mix for display)
    const int numSamples = buffer.getNumSamples();

    if (buffer.getNumChannels() >= 2)
    {
        // Average L+R for analysis
        const float* left  = buffer.getReadPointer(0);
        const float* right = buffer.getReadPointer(1);

        // Use a small stack buffer to avoid allocations
        constexpr int maxChunk = 2048;
        float mono[maxChunk];

        int offset = 0;
        while (offset < numSamples)
        {
            int chunk = juce::jmin(numSamples - offset, maxChunk);
            for (int s = 0; s < chunk; ++s)
                mono[s] = (left[offset + s] + right[offset + s]) * 0.5f;

            analyzer.pushSamples(mono, chunk);
            offset += chunk;
        }
    }
    else
    {
        analyzer.pushSamples(buffer.getReadPointer(0), numSamples);
    }
}

double FisEQAudioProcessor::getCompositeMagnitude(double frequency) const
{
    double magnitude = 1.0;
    for (const auto& band : bands)
        magnitude *= band.getMagnitudeAtFrequency(frequency);
    return magnitude;
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
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FisEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FisEQAudioProcessor();
}
