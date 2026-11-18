// Renders the plugin editor to a PNG, for comparing the faceplate with the
// reference photographs.
//
//   jb160_render_panel <out.png> [width] [name=value ...]
//
// Names: threshold, ratio, outputGain (rotation 0..1), overEasy, bypass, slave,
// display (0/1), level, reduction, over (meter values in dB).

#include "../Source/PluginEditor.h"

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI gui;

    if (argc < 2)
    {
        std::puts ("usage: jb160_render_panel <out.png> [width] [name=value ...]");
        return 2;
    }

    const juce::File out = juce::File::getCurrentWorkingDirectory().getChildFile (juce::String::fromUTF8 (argv[1]));
    int width = 3588;

    JB160XTAudioProcessor processor;
    float level = -100.0f, reduction = 0.0f, over = -100.0f;

    for (int i = 2; i < argc; ++i)
    {
        const juce::String arg (argv[i]);

        if (! arg.containsChar ('='))
        {
            width = arg.getIntValue();
            continue;
        }

        const auto name = arg.upToFirstOccurrenceOf ("=", false, false);
        const auto value = arg.fromFirstOccurrenceOf ("=", false, false).getFloatValue();

        if (name == "level") level = value;
        else if (name == "reduction") reduction = value;
        else if (name == "over") over = value;
        else if (auto* p = processor.apvts.getParameter (name)) p->setValueNotifyingHost (value);
        else std::printf ("unknown setting: %s\n", name.toRawUTF8());
    }

    // Smoke test of the real audio path: one second of a -8 dBFS 1 kHz sine.
    {
        processor.setRateAndBufferSizeDetails (48000.0, 512);
        processor.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> buffer (processor.getTotalNumInputChannels(), 512);
        buffer.clear();
        juce::MidiBuffer midi;
        double phase = 0.0, peak = 0.0;

        for (int block = 0; block < 94; ++block)
        {
            for (int n = 0; n < 512; ++n)
            {
                const auto s = (float) (0.4 * std::sin (phase));
                phase += juce::MathConstants<double>::twoPi * 1000.0 / 48000.0;
                buffer.setSample (0, n, s);
                buffer.setSample (1, n, s);
            }

            processor.processBlock (buffer, midi);
            peak = buffer.getMagnitude (0, 512);
        }

        std::printf ("audio path: in peak 0.400 -> out peak %.3f, meter %.1f dB, reduction %.1f dB, over threshold %.1f dB\n",
                     peak,
                     (double) processor.compressor.meters.levelDb.load(),
                     (double) processor.compressor.meters.gainReductionDb.load(),
                     (double) processor.compressor.meters.overThresholdDb.load());
    }

    processor.compressor.meters.levelDb = level;
    processor.compressor.meters.gainReductionDb = reduction;
    processor.compressor.meters.overThresholdDb = over;

    std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
    editor->setSize (width, juce::roundToInt ((float) width * panel::height / panel::width));

    if (auto* e = dynamic_cast<JB160XTAudioProcessorEditor*> (editor.get()))
        e->refreshIndicators();

    const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);

    out.deleteFile();
    juce::FileOutputStream stream (out);
    if (! stream.openedOk() || ! juce::PNGImageFormat().writeImageToStream (image, stream))
    {
        std::puts ("could not write image");
        return 1;
    }

    std::printf ("wrote %s (%d x %d)\n", out.getFullPathName().toRawUTF8(), image.getWidth(), image.getHeight());
    return 0;
}
