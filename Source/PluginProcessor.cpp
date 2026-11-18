#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
/** Finds the rotation whose (monotonic) law value is closest to the target. */
template <typename Law>
float rotationFor (Law&& law, double target)
{
    double lo = 0.0, hi = 1.0;
    const bool rising = law (1.0) > law (0.0);

    for (int i = 0; i < 40; ++i)
    {
        const double mid = 0.5 * (lo + hi);
        if ((law (mid) < target) == rising) lo = mid; else hi = mid;
    }

    return (float) (0.5 * (lo + hi));
}

double degreesToRotation (double degrees) { return (degrees + 150.0) / 300.0; }
}

//==============================================================================
juce::String JB160XTAudioProcessor::thresholdText (float rotation)
{
    return juce::String (jb160::PanelLaws::thresholdDbu (rotation), 1) + " dBm";
}

juce::String JB160XTAudioProcessor::gainText (float rotation)
{
    const auto db = jb160::PanelLaws::outputGainDb (rotation);
    return (db > 0.0 ? "+" : "") + juce::String (db, 1) + " dB";
}

juce::String JB160XTAudioProcessor::ratioText (float rotation)
{
    const auto ratio = jb160::PanelLaws::ratioFromSlope (jb160::PanelLaws::compressionSlope (rotation));

    if (std::isinf (ratio) || std::abs (ratio) > 200.0)
        return juce::String::fromUTF8 ("\xe2\x88\x9e:1");

    return juce::String (ratio, std::abs (ratio) < 10.0 ? 2 : 1) + ":1";
}

juce::AudioProcessorValueTreeState::ParameterLayout JB160XTAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const NormalisableRange<float> rotation (0.0f, 1.0f, 0.0f);

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::threshold, 1 }, "Threshold", rotation,
        (float) degreesToRotation (50.0), // the "0" mark
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return thresholdText (v); })
            .withValueFromStringFunction ([] (const String& s) { return rotationFor (jb160::PanelLaws::thresholdDbu, s.getDoubleValue()); })));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::ratio, 1 }, "Compression Ratio", rotation,
        (float) degreesToRotation (-24.0), // the "4:1" mark
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return ratioText (v); })
            .withValueFromStringFunction ([] (const String& s)
            {
                const auto t = s.trim().toLowerCase();
                if (t.startsWith ("inf") || t.containsChar (0x221e))
                    return (float) jb160::PanelLaws::infinityMarkRotation;

                const double r = t.getDoubleValue();
                if (std::abs (r) < 1.0e-6)
                    return 0.0f;

                // slope c = 1 - 1/ratio, rising monotonically with rotation
                return rotationFor (jb160::PanelLaws::compressionSlope, 1.0 - 1.0 / r);
            })));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::outputGain, 1 }, "Output Gain", rotation, 0.5f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return gainText (v); })
            .withValueFromStringFunction ([] (const String& s) { return rotationFor (jb160::PanelLaws::outputGainDb, s.getDoubleValue()); })));

    layout.add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::overEasy, 1 }, "Over Easy", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::bypass, 1 }, "Bypass", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::slave, 1 }, "Slave", false));
    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { ParamIDs::display, 1 }, "Display", false,
        AudioParameterBoolAttributes().withStringFromValueFunction ([] (bool on, int) { return on ? "Input" : "Output"; })));

    // Rear panel: a plug in the DETECTOR INPUT jack. Fed by the host's sidechain.
    layout.add (std::make_unique<AudioParameterBool> (
        ParameterID { ParamIDs::detector, 1 }, "Detector Input", false,
        AudioParameterBoolAttributes().withStringFromValueFunction ([] (bool on, int) { return on ? "External (sidechain)" : "Internal"; })));

    return layout;
}

//==============================================================================
JB160XTAudioProcessor::JB160XTAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "JB160XT", createLayout())
{
    thresholdValue = apvts.getRawParameterValue (ParamIDs::threshold);
    ratioValue = apvts.getRawParameterValue (ParamIDs::ratio);
    gainValue = apvts.getRawParameterValue (ParamIDs::outputGain);
    overEasyValue = apvts.getRawParameterValue (ParamIDs::overEasy);
    bypassValue = apvts.getRawParameterValue (ParamIDs::bypass);
    slaveValue = apvts.getRawParameterValue (ParamIDs::slave);
    displayValue = apvts.getRawParameterValue (ParamIDs::display);
    detectorValue = apvts.getRawParameterValue (ParamIDs::detector);
    bypassParam = apvts.getParameter (ParamIDs::bypass);
}

bool JB160XTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in != out || (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo()))
        return false;

    const auto sidechain = layouts.getChannelSet (true, 1);
    return sidechain.isDisabled() || sidechain == juce::AudioChannelSet::mono() || sidechain == juce::AudioChannelSet::stereo();
}

jb160::PanelState JB160XTAudioProcessor::readPanel() const
{
    jb160::PanelState p;
    p.threshold = thresholdValue->load (std::memory_order_relaxed);
    p.ratio = ratioValue->load (std::memory_order_relaxed);
    p.outputGain = gainValue->load (std::memory_order_relaxed);
    p.overEasy = overEasyValue->load (std::memory_order_relaxed) > 0.5f;
    p.bypass = bypassValue->load (std::memory_order_relaxed) > 0.5f;
    p.slave = slaveValue->load (std::memory_order_relaxed) > 0.5f;
    p.displayInput = displayValue->load (std::memory_order_relaxed) > 0.5f;
    p.externalDetector = detectorValue->load (std::memory_order_relaxed) > 0.5f;
    return p;
}

void JB160XTAudioProcessor::prepareToPlay (double sampleRate, int)
{
    compressor.prepare (sampleRate, juce::jlimit (1, 2, getMainBusNumInputChannels()));
    compressor.reset();
    compressor.setPanel (readPanel());
}

void JB160XTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto mainBuffer = getBusBuffer (buffer, true, 0);
    const int numIn = juce::jmin (2, mainBuffer.getNumChannels());

    if (numIn < 1)
        return;

    // The sidechain bus shares the host buffer after the main channels. It is
    // only read, in step with the main input, so processing in place is safe.
    const auto sidechain = getBusCount (true) > 1 ? getBusBuffer (buffer, true, 1) : juce::AudioBuffer<float>();
    const int numSidechain = juce::jmin (2, sidechain.getNumChannels());

    compressor.setPanel (readPanel());

    float* channels[jb160::Compressor160XT::maxChannels] = { mainBuffer.getWritePointer (0),
                                                              numIn > 1 ? mainBuffer.getWritePointer (1) : nullptr };
    const float* detector[jb160::Compressor160XT::maxChannels] = { numSidechain > 0 ? sidechain.getReadPointer (0) : nullptr,
                                                                    numSidechain > 1 ? sidechain.getReadPointer (1) : nullptr };

    compressor.process (channels, numSidechain > 0 ? detector : nullptr, numSidechain, numSamples);
}

void JB160XTAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    // The host's bypass is the panel's BYPASS button, so normal processing
    // already passes the input through; keep the detector and meters running
    // as the powered hardware would.
    processBlock (buffer, midi);
}

//==============================================================================
void JB160XTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void JB160XTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* JB160XTAudioProcessor::createEditor()
{
    return new JB160XTAudioProcessorEditor (*this);
}

#if ! JB160_RENDER_TOOL
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JB160XTAudioProcessor();
}
#endif
