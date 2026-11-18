// JB 160XT - front panel artwork, drawn in code.
//
// All geometry is in "panel units": the rectified straight-on photograph of the
// hardware (Documentation/pictures-of-my-160xt/front.jpg) is 3588 x 331 pixels
// for the 19 in x 1.75 in panel, and positions were measured on it directly.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace panel
{
inline constexpr float width = 3588.0f;
inline constexpr float height = 331.0f;

// Pots turn through 300 degrees; angles are degrees clockwise from 12 o'clock.
inline constexpr float sweepDegrees = 300.0f;
inline float rotationToDegrees (float rotation) { return -150.0f + sweepDegrees * rotation; }

inline constexpr float knobY = 152.0f;
inline constexpr float thresholdX = 1277.0f;
inline constexpr float ratioX = 1758.0f;
inline constexpr float gainX = 2082.0f;
inline constexpr float knobSkirtRadius = 57.0f;
inline constexpr float knobCapRadius = 52.0f;

inline constexpr float buttonWidth = 122.0f;
inline constexpr float buttonHeight = 50.0f;
inline constexpr float buttonY = 208.0f; // centre
inline constexpr float bypassX = 817.0f;
inline constexpr float slaveX = 986.0f;
inline constexpr float overEasyX = 1527.0f;
inline constexpr float displayX = 2287.0f;

inline constexpr float ledRadius = 9.0f;
inline constexpr float meterX0 = 2430.0f;
inline constexpr float meterPitch = 35.55f;
inline constexpr float levelRowY = 113.0f;
inline constexpr float reductionRowY = 180.0f;

inline constexpr int numLevelLeds = 19;
inline constexpr int numReductionLeds = 12;

/** dB value (re the meter's 0) at which each LEVEL LED lights. */
inline constexpr float levelLedDb[numLevelLeds] = { -40, -30, -20, -15, -10, -8, -6, -5, -4, -3, -2, -1, 0, 1, 2, 4, 6, 10, 20 };
/** dB of gain reduction at which each GAIN REDUCTION LED lights (left to right). */
inline constexpr float reductionLedDb[numReductionLeds] = { 40, 30, 20, 15, 10, 8, 6, 5, 4, 3, 2, 1 };

enum class LedColour { red, amber, green };

struct Indicators
{
    bool bypass = false, slave = false, displayInput = false;
    float below = 1.0f, overEasy = 0.0f, above = 0.0f;
    float level[numLevelLeds] {};
    float reduction[numReductionLeds] {};
};

enum class KnobColour { red, blue, green };

class Faceplate
{
public:
    Faceplate();

    /** Returns the static panel (metalwork, lettering, unlit LED lenses, button
        wells) rendered at the given pixel size, cached between calls. */
    const juce::Image& getBackground (int pixelWidth, int pixelHeight);

    /** Draws lit LEDs. The graphics context must be in panel units. */
    void drawIndicators (juce::Graphics&, const Indicators&) const;

    /** Draws a knob centred on the origin of a context in panel units. */
    static void drawKnob (juce::Graphics&, KnobColour, float rotation);

    /** Draws a push button cap in a context in panel units; bounds is the well. */
    static void drawButton (juce::Graphics&, juce::Rectangle<float> bounds, bool pressedIn, bool mouseDown);

    static juce::Rectangle<float> buttonBounds (float centreX)
    {
        return juce::Rectangle<float> (buttonWidth, buttonHeight).withCentre ({ centreX, buttonY });
    }

private:
    void renderBackground (juce::Image&, juce::Graphics&, int pixelWidth, int pixelHeight) const;
    void drawLettering (juce::Graphics&) const;
    void drawLogo (juce::Graphics&) const;
    void drawTicks (juce::Graphics&) const;

    /** Builds the outline of a legend and fits its ink to the given box. */
    juce::Path legend (const juce::String& text, juce::Rectangle<float> box) const;
    juce::Path rawLegend (const juce::String& text) const;
    void drawLegend (juce::Graphics&, const juce::String& text, float x0, float y0, float x1, float y1) const;

    static void drawLed (juce::Graphics&, juce::Point<float> centre, LedColour, float brightness);

    juce::Typeface::Ptr typeface;
    juce::Image cache;
};
} // namespace panel
