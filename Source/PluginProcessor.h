#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "DSP/Compressor160XT.h"

namespace ParamIDs
{
inline constexpr const char* threshold = "threshold";
inline constexpr const char* ratio = "ratio";
inline constexpr const char* outputGain = "outputGain";
inline constexpr const char* overEasy = "overEasy";
inline constexpr const char* bypass = "bypass";
inline constexpr const char* slave = "slave";
inline constexpr const char* display = "display";
inline constexpr const char* detector = "detector";
}

class JB160XTAudioProcessor final : public juce::AudioProcessor
{
public:
    JB160XTAudioProcessor();
    ~JB160XTAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "JB 160XT"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    jb160::Compressor160XT compressor;

    /** Text shown for a pot rotation, as the circuit model maps it. */
    static juce::String thresholdText (float rotation);
    static juce::String ratioText (float rotation);
    static juce::String gainText (float rotation);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    jb160::PanelState readPanel() const;

    std::atomic<float>* thresholdValue = nullptr;
    std::atomic<float>* ratioValue = nullptr;
    std::atomic<float>* gainValue = nullptr;
    std::atomic<float>* overEasyValue = nullptr;
    std::atomic<float>* bypassValue = nullptr;
    std::atomic<float>* slaveValue = nullptr;
    std::atomic<float>* displayValue = nullptr;
    std::atomic<float>* detectorValue = nullptr;
    juce::AudioProcessorParameter* bypassParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JB160XTAudioProcessor)
};
