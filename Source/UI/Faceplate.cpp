#include "Faceplate.h"

#include <BinaryData.h>

namespace panel
{
namespace
{
const juce::Colour ink (232, 230, 220);
constexpr float seamLeft = 158.0f, seamRight = 3438.0f;

juce::Point<float> polar (float cx, float cy, float radius, float degrees)
{
    const float a = juce::degreesToRadians (degrees);
    return { cx + radius * std::sin (a), cy - radius * std::cos (a) };
}

void fillBrushedMetal (juce::Image& image, int x0, int x1, int baseLevel, juce::int64 seed)
{
    juce::Image::BitmapData data (image, juce::Image::BitmapData::writeOnly);
    juce::Random random (seed);
    const int h = image.getHeight();

    for (int y = 0; y < h; ++y)
    {
        const float t = (float) y / (float) juce::jmax (1, h - 1);
        const float shade = 5.0f * (1.0f - t) - 3.0f * t;      // lit from above
        const float row = (random.nextFloat() - 0.5f) * 11.0f; // grain line
        float streak = 0.0f;

        for (int x = x0; x < x1; ++x)
        {
            streak = streak * 0.965f + (random.nextFloat() - 0.5f) * 3.4f;
            const float v = (float) baseLevel + shade + row + streak + (random.nextFloat() - 0.5f) * 3.0f;
            const auto c = (juce::uint8) juce::jlimit (0.0f, 255.0f, v);
            data.setPixelColour (x, y, juce::Colour (c, c, (juce::uint8) juce::jmax (0, (int) c - 1)));
        }
    }
}
} // namespace

//==============================================================================
Faceplate::Faceplate()
{
    typeface = juce::Typeface::createSystemTypefaceFor (Assets::MichromaRegular_ttf,
                                                        (size_t) Assets::MichromaRegular_ttfSize);
}

const juce::Image& Faceplate::getBackground (int pixelWidth, int pixelHeight)
{
    pixelWidth = juce::jmax (1, pixelWidth);
    pixelHeight = juce::jmax (1, pixelHeight);

    if (cache.isNull() || cache.getWidth() != pixelWidth || cache.getHeight() != pixelHeight)
    {
        // A software image: the metal texture is written pixel by pixel before
        // any drawing context is opened on it.
        cache = juce::Image (juce::Image::RGB, pixelWidth, pixelHeight, false, juce::SoftwareImageType());

        const float scale = (float) pixelWidth / width;
        const int xl = juce::jlimit (0, pixelWidth, juce::roundToInt (seamLeft * scale));
        const int xr = juce::jlimit (0, pixelWidth, juce::roundToInt (seamRight * scale));
        fillBrushedMetal (cache, 0, xl, 46, 11);
        fillBrushedMetal (cache, xl, xr, 40, 23);
        fillBrushedMetal (cache, xr, pixelWidth, 46, 37);

        juce::Graphics g (cache);
        renderBackground (cache, g, pixelWidth, pixelHeight);
    }

    return cache;
}

//==============================================================================
juce::Path Faceplate::rawLegend (const juce::String& text) const
{
    const juce::Font font (juce::FontOptions().withTypeface (typeface).withHeight (100.0f));

    // Cap height of the face at this size, from the ink of a flat-topped letter.
    float capHeight = 70.0f;
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, "H", 0.0f, 0.0f);
        juce::Path h;
        ga.createPath (h);
        if (! h.isEmpty())
            capHeight = h.getBounds().getHeight();
    }

    juce::Path result;
    float x = 0.0f;
    juce::String run;

    const auto flush = [&]
    {
        if (run.isEmpty())
            return;

        juce::GlyphArrangement ga;
        ga.addLineOfText (font, run, x, 0.0f);
        juce::Path p;
        ga.createPath (p);
        result.addPath (p);
        x += juce::GlyphArrangement::getStringWidth (font, run);
        run.clear();
    };

    for (auto c : text)
    {
        if (c == '-')
        {
            // The panel uses a true minus, wider than the face's hyphen.
            flush();
            const float w = capHeight * 0.62f, t = capHeight * 0.19f;
            result.addRectangle (x + capHeight * 0.06f, -capHeight * 0.5f - t * 0.5f, w, t);
            x += w + capHeight * 0.20f;
        }
        else if (c == 0x221e)
        {
            flush();
            const float w = capHeight * 1.55f, h = capHeight * 0.74f, t = capHeight * 0.18f;
            juce::Path loops;
            loops.addEllipse (x + t * 0.5f, -capHeight * 0.5f - h * 0.5f + t * 0.5f, w * 0.5f - t * 0.5f, h - t);
            loops.addEllipse (x + w * 0.5f, -capHeight * 0.5f - h * 0.5f + t * 0.5f, w * 0.5f - t * 0.5f, h - t);
            juce::Path stroked;
            juce::PathStrokeType (t).createStrokedPath (stroked, loops);
            result.addPath (stroked);
            x += w + capHeight * 0.14f;
        }
        else
        {
            run += c;
        }
    }

    flush();
    return result;
}

juce::Path Faceplate::legend (const juce::String& text, juce::Rectangle<float> box) const
{
    auto p = rawLegend (text);
    const auto b = p.getBounds();

    if (b.getWidth() <= 0.0f || b.getHeight() <= 0.0f)
        return {};

    p.applyTransform (juce::AffineTransform::translation (-b.getX(), -b.getY())
                          .scaled (box.getWidth() / b.getWidth(), box.getHeight() / b.getHeight())
                          .translated (box.getX(), box.getY()));
    return p;
}

void Faceplate::drawLegend (juce::Graphics& g, const juce::String& text, float x0, float y0, float x1, float y1) const
{
    // The panel lettering is a bold extended face; the regular-weight outline is
    // thickened with a stroke, and the box is inset so the ink still lands on
    // the measured extents.
    const float bold = (y1 - y0) * 0.115f;
    const auto p = legend (text, juce::Rectangle<float> (x0, y0, x1 - x0, y1 - y0).reduced (bold * 0.5f));

    g.setColour (ink);
    g.fillPath (p);
    g.strokePath (p, juce::PathStrokeType (bold, juce::PathStrokeType::curved));
}

//==============================================================================
void Faceplate::drawLogo (juce::Graphics& g) const
{
    // "JB" occupies the box where the hardware carries its maker's logo.
    drawLegend (g, "JB", 384.0f, 78.0f, 534.0f, 181.0f);
    drawLegend (g, "160XT", 338.0f, 192.0f, 580.0f, 232.0f);
}

void Faceplate::drawTicks (juce::Graphics& g) const
{
    g.setColour (ink);

    const auto tick = [&] (float cx, float degrees)
    {
        g.drawLine ({ polar (cx, knobY, 63.0f, degrees), polar (cx, knobY, 77.0f, degrees) }, 3.6f);
    };

    for (int i = 0; i < 7; ++i) tick (thresholdX, -150.0f + 50.0f * (float) i);
    for (int i = 0; i < 5; ++i) tick (gainX, -150.0f + 75.0f * (float) i);

    // Ratio marks as printed: 1:1, 2:1, 3:1, 4:1, 6:1, 10:1, inf:1, -5:1, -2:1, -1:1
    for (float degrees : { -150.0f, -100.0f, -55.0f, -24.0f, 18.0f, 44.0f, 75.0f, 103.0f, 126.0f, 150.0f })
        tick (ratioX, degrees);
}

void Faceplate::drawLettering (juce::Graphics& g) const
{
    const auto inf = juce::String::charToString ((juce::juce_wchar) 0x221e);

    drawLegend (g, "BYPASS", 750, 269, 882, 288);
    drawLegend (g, "SLAVE", 937, 269, 1043, 288);

    // Threshold knob
    drawLegend (g, "-40dBm", 1162, 218, 1262, 236);
    drawLegend (g, "-30", 1150, 156, 1195, 172);
    drawLegend (g, "-20", 1160, 84, 1210, 100);
    drawLegend (g, "-10", 1247, 52, 1290, 68);
    drawLegend (g, "0", 1336, 84, 1353, 100);
    drawLegend (g, "+10", 1360, 156, 1402, 172);
    drawLegend (g, "+20dBm", 1296, 218, 1396, 236);
    drawLegend (g, "THRESHOLD", 1185, 268, 1380, 288);

    // Threshold indicators
    drawLegend (g, "THRESHOLD", 1420, 46, 1614, 66);
    drawLegend (g, "BELOW", 1418, 122, 1508, 138);
    drawLegend (g, "ABOVE", 1530, 122, 1620, 138);
    drawLegend (g, "OVER EASY", 1434, 268, 1612, 288);

    // Ratio knob
    drawLegend (g, "1:1", 1700, 222, 1730, 238);
    drawLegend (g, "2:1", 1648, 156, 1683, 172);
    drawLegend (g, "3:1", 1660, 92, 1695, 108);
    drawLegend (g, "4:1", 1705, 55, 1740, 72);
    drawLegend (g, "6:1", 1770, 55, 1805, 72);
    drawLegend (g, "10:1", 1815, 84, 1860, 100);
    drawLegend (g, inf + ":1", 1838, 122, 1873, 138);
    drawLegend (g, "-5:1", 1838, 158, 1885, 174);
    drawLegend (g, "-2:1", 1830, 194, 1875, 210);
    drawLegend (g, "-1:1", 1775, 222, 1818, 238);
    drawLegend (g, "COMPRESSION", 1642, 254, 1875, 272);
    drawLegend (g, "RATIO", 1710, 284, 1806, 302);

    // Output gain knob
    drawLegend (g, "-20dB", 1992, 222, 2068, 238);
    drawLegend (g, "-10", 1965, 118, 2008, 134);
    drawLegend (g, "0", 2074, 60, 2090, 76);
    drawLegend (g, "+10", 2155, 118, 2195, 134);
    drawLegend (g, "+20dB", 2103, 222, 2175, 238);
    drawLegend (g, "OUTPUT", 2022, 256, 2140, 274);
    drawLegend (g, "GAIN", 2048, 284, 2118, 302);

    // Display select
    drawLegend (g, "INPUT", 2244, 50, 2318, 66);
    drawLegend (g, "OUTPUT", 2232, 156, 2328, 172);
    drawLegend (g, "DISPLAY", 2220, 268, 2342, 288);

    g.setColour (ink);
    g.fillRect (juce::Rectangle<float> (2298.0f, 93.5f, 66.0f, 3.2f));
    g.fillRect (juce::Rectangle<float> (2298.0f, 127.5f, 66.0f, 3.2f));
    g.fillRect (juce::Rectangle<float> (2361.0f, 93.5f, 3.2f, 37.2f));
    g.fillRect (juce::Rectangle<float> (2361.0f, 111.4f, 58.0f, 3.2f));

    // Meter scale: the same legend serves both rows.
    const char* numbers[numLevelLeds] = { "40", "30", "20", "15", "10", "8", "6", "5", "4", "3", "2", "1", "0", "1", "2", "4", "6", "10", "20" };

    for (int i = 0; i < numLevelLeds; ++i)
    {
        const float cx = meterX0 + meterPitch * (float) i;
        const juce::String text (numbers[i]);
        const float half = text.length() == 1 ? (text == "1" ? 5.0f : 7.5f) : 14.0f;
        drawLegend (g, text, cx - half, 139, cx + half, 155);

        if (i == 0)
            g.fillRect (juce::Rectangle<float> (cx - half - 15.0f, 145.4f, 11.0f, 3.4f)); // "-40"
        if (i == numLevelLeds - 1)
            drawLegend (g, "+", cx + half + 3.0f, 141.5f, cx + half + 14.0f, 152.5f);     // "20+"
    }

    g.setColour (ink);
    g.fillRect (juce::Rectangle<float> (2838.0f, 177.6f, 110.0f, 3.2f));
    drawLegend (g, "LEVEL", 3110, 104, 3200, 124);
    drawLegend (g, "GAIN REDUCTION", 2954, 168, 3204, 188);
    drawLegend (g, "COMPRESSOR/LIMITER", 2536, 244, 2952, 264);
}

//==============================================================================
void Faceplate::renderBackground (juce::Image&, juce::Graphics& g, int pixelWidth, int pixelHeight) const
{
    const float scale = (float) pixelWidth / width;
    g.addTransform (juce::AffineTransform::scale (scale, (float) pixelHeight / height));

    // Extrusion steps along the top and bottom of the panel.
    for (float y : { 12.0f, 316.0f })
    {
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.fillRect (juce::Rectangle<float> (seamLeft, y, seamRight - seamLeft, 3.5f));
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.fillRect (juce::Rectangle<float> (seamLeft, y + 3.5f, seamRight - seamLeft, 1.5f));
    }

    for (float x : { seamLeft, seamRight })
    {
        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRect (juce::Rectangle<float> (x - 3.0f, 0.0f, 6.0f, height));
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRect (juce::Rectangle<float> (x + 3.0f, 0.0f, 1.5f, height));
    }

    // Rack slots.
    for (float cx : { 60.0f, width - 60.0f })
        for (float cy : { 47.0f, 284.0f })
        {
            const auto slot = juce::Rectangle<float> (78.0f, 52.0f).withCentre ({ cx, cy });
            g.setColour (juce::Colour (150, 148, 140).withAlpha (0.45f));
            g.fillRoundedRectangle (slot.translated (0.0f, 2.5f), 26.0f);
            g.setColour (juce::Colour (5, 5, 5));
            g.fillRoundedRectangle (slot, 26.0f);
        }

    // Logo label: a slightly rougher, darker textured patch.
    {
        const juce::Rectangle<float> patch (236.0f, 32.0f, 402.0f, 248.0f);
        g.setColour (juce::Colour (18, 18, 17).withAlpha (0.5f));
        g.fillRoundedRectangle (patch, 13.0f);

        juce::Graphics::ScopedSaveState state (g);
        juce::Path clip;
        clip.addRoundedRectangle (patch, 13.0f);
        g.reduceClipRegion (clip);

        juce::Random random (4242);
        for (int i = 0; i < 5200; ++i)
        {
            const float x = patch.getX() + random.nextFloat() * patch.getWidth();
            const float y = patch.getY() + random.nextFloat() * patch.getHeight();
            const float len = 1.5f + random.nextFloat() * 4.0f;
            const float a = random.nextFloat() * juce::MathConstants<float>::twoPi;
            g.setColour (juce::Colours::white.withAlpha (0.025f + random.nextFloat() * 0.055f));
            g.drawLine (x, y, x + len * std::cos (a), y + len * std::sin (a), 1.0f);
        }
    }

    drawLogo (g);
    drawTicks (g);
    drawLettering (g);

    // Button wells.
    for (float cx : { bypassX, slaveX, overEasyX, displayX })
    {
        g.setColour (juce::Colour (4, 4, 4));
        g.fillRect (buttonBounds (cx));
    }

    // Unlit LED lenses.
    drawIndicators (g, Indicators { .below = 0.0f });
}

//==============================================================================
void Faceplate::drawLed (juce::Graphics& g, juce::Point<float> c, LedColour colour, float brightness)
{
    brightness = juce::jlimit (0.0f, 1.0f, brightness);

    juce::Colour off, on;
    switch (colour)
    {
        case LedColour::red:   off = juce::Colour (104, 24, 26);  on = juce::Colour (255, 52, 40);  break;
        case LedColour::amber: off = juce::Colour (150, 118, 58); on = juce::Colour (255, 196, 64); break;
        case LedColour::green: off = juce::Colour (20, 86, 60);   on = juce::Colour (64, 255, 132); break;
    }

    const float r = ledRadius;

    if (brightness > 0.0f)
    {
        juce::ColourGradient glow (on.withAlpha (0.55f * brightness), c.x, c.y, on.withAlpha (0.0f), c.x + r * 3.2f, c.y, true);
        g.setGradientFill (glow);
        g.fillEllipse (juce::Rectangle<float> (r * 6.4f, r * 6.4f).withCentre (c));
    }

    g.setColour (juce::Colour (6, 6, 6));
    g.fillEllipse (juce::Rectangle<float> (2.0f * r + 3.0f, 2.0f * r + 3.0f).withCentre (c));

    const auto body = off.interpolatedWith (on, brightness);
    juce::ColourGradient lens (body.brighter (0.25f + 0.5f * brightness), c.x - r * 0.35f, c.y - r * 0.4f,
                               body.darker (0.35f * (1.0f - brightness)), c.x + r, c.y + r, true);
    g.setGradientFill (lens);
    g.fillEllipse (juce::Rectangle<float> (2.0f * r, 2.0f * r).withCentre (c));

    g.setColour (juce::Colours::white.withAlpha (0.16f + 0.5f * brightness));
    g.fillEllipse (juce::Rectangle<float> (r * 0.7f, r * 0.5f).withCentre ({ c.x - r * 0.3f, c.y - r * 0.38f }));
}

void Faceplate::drawIndicators (juce::Graphics& g, const Indicators& s) const
{
    drawLed (g, { 808.0f, 90.0f }, LedColour::red, s.bypass ? 1.0f : 0.0f);
    drawLed (g, { 985.0f, 90.0f }, LedColour::amber, s.slave ? 1.0f : 0.0f);

    drawLed (g, { 1466.0f, 93.0f }, LedColour::green, s.below);
    drawLed (g, { 1522.0f, 93.0f }, LedColour::amber, s.overEasy);
    drawLed (g, { 1580.0f, 93.0f }, LedColour::red, s.above);

    drawLed (g, { displayX, 97.0f }, LedColour::red, s.displayInput ? 1.0f : 0.0f);
    drawLed (g, { displayX, 129.0f }, LedColour::red, s.displayInput ? 0.0f : 1.0f);

    for (int i = 0; i < numLevelLeds; ++i)
        drawLed (g, { meterX0 + meterPitch * (float) i, levelRowY }, i < 12 ? LedColour::amber : LedColour::red, s.level[i]);

    for (int i = 0; i < numReductionLeds; ++i)
        drawLed (g, { meterX0 + meterPitch * (float) i, reductionRowY }, LedColour::red, s.reduction[i]);
}

//==============================================================================
void Faceplate::drawKnob (juce::Graphics& g, KnobColour colour, float rotation)
{
    const float degrees = rotationToDegrees (juce::jlimit (0.0f, 1.0f, rotation));
    const float rs = knobSkirtRadius, rc = knobCapRadius;

    // Cast shadow, down and to the right as in the reference photographs.
    {
        juce::ColourGradient shadow (juce::Colours::black.withAlpha (0.62f), 5.0f, 9.0f,
                                     juce::Colours::transparentBlack, 5.0f + rs + 16.0f, 9.0f, true);
        shadow.addColour (0.72, juce::Colours::black.withAlpha (0.5f));
        g.setGradientFill (shadow);
        g.fillEllipse (juce::Rectangle<float> (2.0f * (rs + 16.0f), 2.0f * (rs + 16.0f)).withCentre ({ 5.0f, 9.0f }));
    }

    // Fluted black skirt.
    g.setColour (juce::Colour (17, 17, 17));
    g.fillEllipse (juce::Rectangle<float> (2.0f * rs, 2.0f * rs).withCentre ({}));

    for (int i = 0; i < 18; ++i)
    {
        const float a = degrees + 10.0f + 20.0f * (float) i;
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawLine ({ polar (0, 0, rc - 1.0f, a), polar (0, 0, rs - 0.5f, a) }, 2.2f);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawLine ({ polar (0, 0, rc - 1.0f, a + 4.0f), polar (0, 0, rs - 0.5f, a + 4.0f) }, 1.6f);
    }

    // Pointer line runs down the side of the skirt too.
    g.setColour (ink);
    g.drawLine ({ polar (0, 0, rc - 2.0f, degrees), polar (0, 0, rs - 0.5f, degrees) }, 6.4f);

    // Coloured cap.
    juce::Colour cap;
    switch (colour)
    {
        case KnobColour::red:   cap = juce::Colour (212, 30, 26);  break;
        case KnobColour::blue:  cap = juce::Colour (22, 138, 194); break;
        case KnobColour::green: cap = juce::Colour (26, 144, 102); break;
    }

    g.setColour (juce::Colour (10, 10, 10));
    g.fillEllipse (juce::Rectangle<float> (2.0f * rc + 2.5f, 2.0f * rc + 2.5f).withCentre ({}));

    juce::ColourGradient face (cap.brighter (0.16f), -rc * 0.45f, -rc * 0.55f, cap.darker (0.14f), rc * 0.8f, rc * 0.9f, true);
    g.setGradientFill (face);
    g.fillEllipse (juce::Rectangle<float> (2.0f * rc, 2.0f * rc).withCentre ({}));

    g.setColour (juce::Colours::black.withAlpha (0.16f));
    g.drawEllipse (juce::Rectangle<float> (2.0f * rc - 2.0f, 2.0f * rc - 2.0f).withCentre ({}), 2.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle (-3.3f, -rc + 0.5f, 6.6f, rc - 7.0f, 1.2f);
    g.setColour (ink);
    g.fillPath (pointer, juce::AffineTransform::rotation (juce::degreesToRadians (degrees)));
}

void Faceplate::drawButton (juce::Graphics& g, juce::Rectangle<float> well, bool pressedIn, bool mouseDown)
{
    g.setColour (juce::Colour (4, 4, 4));
    g.fillRect (well);

    if (! pressedIn)
    {
        // Proud of the panel: casts a shadow below and catches light on top.
        juce::ColourGradient shadow (juce::Colours::black.withAlpha (0.7f), well.getCentreX(), well.getBottom(),
                                     juce::Colours::transparentBlack, well.getCentreX(), well.getBottom() + 20.0f, false);
        g.setGradientFill (shadow);
        g.fillRect (well.withTop (well.getBottom()).withHeight (20.0f).reduced (4.0f, 0.0f));

        const auto cap = well.reduced (3.0f, 3.0f);
        const float lift = mouseDown ? 0.0f : 6.0f;
        juce::ColourGradient body (juce::Colour (64, 64, 62).darker (mouseDown ? 0.3f : 0.0f), cap.getX(), cap.getY(),
                                   juce::Colour (26, 26, 25), cap.getX(), cap.getBottom(), false);
        g.setGradientFill (body);
        g.fillRect (cap);

        g.setColour (juce::Colour (150, 148, 138).withAlpha (mouseDown ? 0.25f : 0.5f));
        g.fillRect (cap.withHeight (3.0f + lift * 0.5f));
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRect (cap.withTop (cap.getBottom() - 4.0f));
    }
    else
    {
        // Latched in: sits below the panel surface inside the well.
        const auto cap = well.reduced (7.0f, 7.0f);
        juce::ColourGradient body (juce::Colour (30, 30, 29), cap.getX(), cap.getY(),
                                   juce::Colour (16, 16, 16), cap.getX(), cap.getBottom(), false);
        g.setGradientFill (body);
        g.fillRect (cap);

        juce::ColourGradient inner (juce::Colours::black.withAlpha (0.85f), well.getX(), well.getY(),
                                    juce::Colours::transparentBlack, well.getX(), well.getY() + 18.0f, false);
        g.setGradientFill (inner);
        g.fillRect (well.withHeight (18.0f));

        g.setColour (juce::Colour (90, 89, 84).withAlpha (0.35f));
        g.fillRect (cap.withHeight (1.6f));
    }
}
} // namespace panel
