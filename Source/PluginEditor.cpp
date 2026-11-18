#include "PluginEditor.h"

namespace
{
constexpr int defaultWidth = 1500;

int heightForWidth (int w) { return juce::roundToInt ((float) w * panel::height / panel::width); }

/** Comparator-driven LED: on above its threshold, with a small soft zone so a
    level sitting on the threshold does not flicker between frames. */
float ledDrive (float value, float threshold)
{
    return juce::jlimit (0.0f, 1.0f, (value - threshold) / 0.3f + 0.5f);
}
}

JB160XTAudioProcessorEditor::JB160XTAudioProcessorEditor (JB160XTAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setOpaque (true);

    for (auto* knob : { &thresholdKnob, &ratioKnob, &gainKnob })
    {
        addAndMakeVisible (*knob);
        knob->setPopupDisplayEnabled (true, false, this);
    }

    for (auto* button : { &bypassButton, &slaveButton, &overEasyButton, &displayButton })
        addAndMakeVisible (*button);

    auto& state = processor.apvts;
    thresholdAttachment = std::make_unique<SliderAttachment> (state, ParamIDs::threshold, thresholdKnob);
    ratioAttachment = std::make_unique<SliderAttachment> (state, ParamIDs::ratio, ratioKnob);
    gainAttachment = std::make_unique<SliderAttachment> (state, ParamIDs::outputGain, gainKnob);
    bypassAttachment = std::make_unique<ButtonAttachment> (state, ParamIDs::bypass, bypassButton);
    slaveAttachment = std::make_unique<ButtonAttachment> (state, ParamIDs::slave, slaveButton);
    overEasyAttachment = std::make_unique<ButtonAttachment> (state, ParamIDs::overEasy, overEasyButton);
    displayAttachment = std::make_unique<ButtonAttachment> (state, ParamIDs::display, displayButton);

    thresholdKnob.setDoubleClickReturnValue (true, state.getParameter (ParamIDs::threshold)->getDefaultValue());
    ratioKnob.setDoubleClickReturnValue (true, state.getParameter (ParamIDs::ratio)->getDefaultValue());
    gainKnob.setDoubleClickReturnValue (true, state.getParameter (ParamIDs::outputGain)->getDefaultValue());

    setResizable (true, false);
    getConstrainer()->setFixedAspectRatio ((double) panel::width / (double) panel::height);
    setResizeLimits (900, heightForWidth (900), 3588, heightForWidth (3588));
    setSize (defaultWidth, heightForWidth (defaultWidth));

    refreshIndicators();
    startTimerHz (40);
}

JB160XTAudioProcessorEditor::~JB160XTAudioProcessorEditor()
{
    stopTimer();
}

void JB160XTAudioProcessorEditor::resized()
{
    const float scale = (float) getWidth() / panel::width;

    const auto place = [scale] (juce::Rectangle<float> r) { return (r * scale).getSmallestIntegerContainer(); };

    const float knobBox = 2.0f * (panel::knobSkirtRadius + 30.0f);
    const auto knobBounds = [&] (float cx)
    {
        // Centred exactly, so the knob's own centre lands on the panel position.
        const auto centre = juce::Point<float> (cx, panel::knobY) * scale;
        const int half = juce::roundToInt (knobBox * 0.5f * scale);
        return juce::Rectangle<int> (2 * half, 2 * half).withCentre (centre.roundToInt());
    };

    for (auto [knob, cx] : { std::pair { &thresholdKnob, panel::thresholdX },
                             std::pair { &ratioKnob, panel::ratioX },
                             std::pair { &gainKnob, panel::gainX } })
    {
        knob->setPanelScale (scale);
        knob->setBounds (knobBounds (cx));
    }

    for (auto [button, cx] : { std::pair { &bypassButton, panel::bypassX },
                               std::pair { &slaveButton, panel::slaveX },
                               std::pair { &overEasyButton, panel::overEasyX },
                               std::pair { &displayButton, panel::displayX } })
    {
        const auto well = panel::Faceplate::buttonBounds (cx);
        const auto pixels = place (well.expanded (6.0f, 0.0f).withTop (well.getY() - 4.0f).withBottom (well.getBottom() + 24.0f));
        button->setPanelGeometry (well, pixels.toFloat() / scale, scale);
        button->setBounds (pixels);
    }
}

void JB160XTAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float pixelScale = g.getInternalContext().getPhysicalPixelScaleFactor();
    const auto& background = faceplate.getBackground (juce::roundToInt ((float) getWidth() * pixelScale),
                                                      juce::roundToInt ((float) getHeight() * pixelScale));

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.drawImage (background, getLocalBounds().toFloat());

    g.addTransform (juce::AffineTransform::scale ((float) getWidth() / panel::width));
    faceplate.drawIndicators (g, indicators);
}

void JB160XTAudioProcessorEditor::refreshIndicators()
{
    const auto& meters = processor.compressor.meters;
    const float level = meters.levelDb.load (std::memory_order_relaxed);
    const float reduction = meters.gainReductionDb.load (std::memory_order_relaxed);
    const float over = meters.overThresholdDb.load (std::memory_order_relaxed);

    const auto isOn = [this] (const char* id) { return processor.apvts.getRawParameterValue (id)->load() > 0.5f; };

    panel::Indicators next;
    next.bypass = isOn (ParamIDs::bypass);
    next.slave = isOn (ParamIDs::slave);
    next.displayInput = isOn (ParamIDs::display);

    // One of the three threshold LEDs is lit at any time (factory step H.15).
    const bool easy = isOn (ParamIDs::overEasy);
    const float amberOnset = (float) jb160::Compressor160XT::overEasyAmberOnsetDb();
    next.above = over >= 0.0f ? 1.0f : 0.0f;
    next.overEasy = (easy && over >= amberOnset && over < 0.0f) ? 1.0f : 0.0f;
    next.below = (next.above + next.overEasy) > 0.0f ? 0.0f : 1.0f;

    for (int i = 0; i < panel::numLevelLeds; ++i)
        next.level[i] = ledDrive (level, panel::levelLedDb[i]);

    for (int i = 0; i < panel::numReductionLeds; ++i)
        next.reduction[i] = ledDrive (reduction, panel::reductionLedDb[i]);

    indicators = next;
}

void JB160XTAudioProcessorEditor::timerCallback()
{
    refreshIndicators();
    repaint();
}

void JB160XTAudioProcessorEditor::mouseDown (const juce::MouseEvent& event)
{
    if (! event.mods.isPopupMenu())
        return;

    // The DETECTOR INPUT jack lives on the rear panel, so it is reached from a
    // context menu and the front panel stays as the hardware has it.
    auto* parameter = processor.apvts.getParameter (ParamIDs::detector);
    const bool external = parameter->getValue() > 0.5f;

    const auto choose = [parameter] (bool useExternal)
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (useExternal ? 1.0f : 0.0f);
        parameter->endChangeGesture();
    };

    juce::PopupMenu menu;
    menu.addSectionHeader ("Detector input (rear panel)");
    menu.addItem ("Internal - follows this track", true, ! external, [choose] { choose (false); });
    menu.addItem ("External - follows the sidechain input", true, external, [choose] { choose (true); });
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition());
}
