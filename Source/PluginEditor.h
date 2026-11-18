#pragma once

#include "PluginProcessor.h"
#include "UI/Faceplate.h"

//==============================================================================
class PanelKnob final : public juce::Slider
{
public:
    explicit PanelKnob (panel::KnobColour c) : colour (c)
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        setMouseDragSensitivity (260);
        setPaintingIsUnclipped (true);
    }

    void setPanelScale (float s) { scale = s; }

    void paint (juce::Graphics& g) override
    {
        g.addTransform (juce::AffineTransform::scale (scale).translated ((float) getWidth() * 0.5f, (float) getHeight() * 0.5f));
        const auto range = getRange();
        const auto rotation = range.getLength() > 0.0 ? (getValue() - range.getStart()) / range.getLength() : 0.0;
        panel::Faceplate::drawKnob (g, colour, (float) rotation);
    }

    bool hitTest (int x, int y) override
    {
        const float dx = (float) x - (float) getWidth() * 0.5f, dy = (float) y - (float) getHeight() * 0.5f;
        const float r = (panel::knobSkirtRadius + 6.0f) * scale;
        return dx * dx + dy * dy <= r * r;
    }

private:
    panel::KnobColour colour;
    float scale = 1.0f;
};

//==============================================================================
class PanelButton final : public juce::Button
{
public:
    PanelButton() : juce::Button ({})
    {
        setClickingTogglesState (true);
    }

    /** well: the button's opening in panel units; the component is larger to
        hold the cast shadow. */
    void setPanelGeometry (juce::Rectangle<float> wellInPanelUnits, juce::Rectangle<float> componentInPanelUnits, float s)
    {
        well = wellInPanelUnits;
        origin = componentInPanelUnits.getPosition();
        scale = s;
    }

    void paintButton (juce::Graphics& g, bool, bool down) override
    {
        g.addTransform (juce::AffineTransform::translation (-origin.x, -origin.y).scaled (scale));
        panel::Faceplate::drawButton (g, well, getToggleState(), down);
    }

    bool hitTest (int x, int y) override
    {
        return well.contains (origin + juce::Point<float> ((float) x, (float) y) / scale);
    }

private:
    juce::Rectangle<float> well;
    juce::Point<float> origin;
    float scale = 1.0f;
};

//==============================================================================
class JB160XTAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit JB160XTAudioProcessorEditor (JB160XTAudioProcessor&);
    ~JB160XTAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    /** Reads the processor's meter values into the LED states. */
    void refreshIndicators();

private:
    void timerCallback() override;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    JB160XTAudioProcessor& processor;
    panel::Faceplate faceplate;
    panel::Indicators indicators;

    PanelKnob thresholdKnob { panel::KnobColour::red };
    PanelKnob ratioKnob { panel::KnobColour::blue };
    PanelKnob gainKnob { panel::KnobColour::green };
    PanelButton bypassButton, slaveButton, overEasyButton, displayButton;

    std::unique_ptr<SliderAttachment> thresholdAttachment, ratioAttachment, gainAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment, slaveAttachment, overEasyAttachment, displayAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JB160XTAudioProcessorEditor)
};
