// JB 160XT - the complete compressor: one or two "units", optionally strapped.
#pragma once

#include "Circuit160XT.h"

#include <array>
#include <atomic>

namespace jb160
{
/** Front-panel state. Pots are rotations, 0 (CCW) to 1 (CW). */
struct PanelState
{
    double threshold = 0.5;
    double ratio = 0.0;
    double outputGain = 0.5;
    bool overEasy = false;
    bool bypass = false;
    bool slave = false;
    bool displayInput = false;
    bool externalDetector = false; // a plug in the rear-panel DETECTOR INPUT jack
};

/** Values for the front-panel indicators, written by the audio thread. */
struct MeterState
{
    std::atomic<float> levelDb { -100.0f };          // re the meter's 0 (= +4 dBu)
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> overThresholdDb { -100.0f };  // detector level minus threshold
};

class Compressor160XT
{
public:
    static constexpr int maxChannels = 2;

    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        channels = std::clamp (numChannels, 1, maxChannels);

        for (auto& u : units)
        {
            u.audioHp.prepare (fs, 1.0 / (2.0 * kPi * 24.9e3 * 1.0e-6));     // C16 1 uF, R8 24k9
            u.detectorHp.prepare (fs, 1.0 / (2.0 * kPi * 34.0e3 * 330.0e-9)); // C37 330 nF, R42 34k0
            u.meterHp.prepare (fs, 1.0 / (2.0 * kPi * 56.2e3 * 220.0e-9));    // C61 220 nF, R111 56k2
            u.timing.prepare (fs);
            u.meter.prepare (fs);
        }

        linkedTiming.prepare (fs);

        // The control voltages are DC and unfiltered in the hardware; this only
        // removes stepping from host automation.
        smoothing = 1.0 - std::exp (-1.0 / (0.005 * fs));
        haveTargets = false;
        meterDecimator = 0;
    }

    void reset()
    {
        for (auto& u : units)
        {
            u.audioHp.reset();
            u.detectorHp.reset();
            u.meterHp.reset();
            u.timing.reset (1.0e-12);
            u.meter.reset();
        }

        linkedTiming.reset (1.0e-12);
    }

    void setPanel (const PanelState& p)
    {
        panel = p;
        targetThreshold = PanelLaws::thresholdDbu (p.threshold);
        targetSlope = PanelLaws::compressionSlope (p.ratio);
        targetGain = PanelLaws::outputGainDb (p.outputGain);

        if (! haveTargets)
        {
            threshold = targetThreshold;
            slope = targetSlope;
            gainDb = targetGain;
            haveTargets = true;
        }
    }

    void setCalibration (const Calibration& c) { calibration = c; }

    /** Processes in place. data[ch] must hold numSamples samples for each of the
        prepared channels. */
    template <typename Sample>
    void process (Sample* const* data, int numSamples)
    {
        process (data, static_cast<const Sample* const*> (nullptr), 0, numSamples);
    }

    /** As above, with a separate detector feed (the DETECTOR INPUT jack). It is
        used when the panel state selects the external detector; a mono feed
        drives both units. With no feed supplied the detector stays normalled
        to the signal input. */
    template <typename Sample>
    void process (Sample* const* data, const Sample* const* detector, int numDetectorChannels, int numSamples)
    {
        const bool external = panel.externalDetector && detector != nullptr && numDetectorChannels > 0;
        const bool stereo = channels == 2;
        const bool strapped = stereo && panel.slave;   // pair with one SLAVE in
        const bool slaveAlone = ! stereo && panel.slave; // SLAVE in, nothing on the strap jack
        const double lnToDb = 10.0 / kLn10;
        const double levelOffset = 3.0103 + calibration.fullScaleSineDbu;

        double maxOver = -200.0, maxGr = 0.0, meterMs = 0.0;

        for (int n = 0; n < numSamples; ++n)
        {
            threshold += smoothing * (targetThreshold - threshold);
            slope += smoothing * (targetSlope - slope);
            gainDb += smoothing * (targetGain - gainDb);

            double in[maxChannels] {}, det[maxChannels] {}, gain[maxChannels] { 1.0, 1.0 };
            double over[maxChannels] { -200.0, -200.0 }, gr[maxChannels] {};

            for (int ch = 0; ch < channels; ++ch)
            {
                in[ch] = (double) data[ch][n];
                const double feed = external ? (double) detector[std::min (ch, numDetectorChannels - 1)][n] : in[ch];
                det[ch] = units[(size_t) ch].detectorHp.process (feed);
            }

            if (strapped)
            {
                // True power summing: both detectors feed one timing node.
                const double L = linkedTiming.process (det[0] * det[0] + det[1] * det[1]);
                over[0] = over[1] = L * lnToDb + levelOffset - threshold;
                gr[0] = gr[1] = slope * knee (over[0]);
                gain[0] = gain[1] = dbToGain (gainDb - gr[0]);
            }
            else
            {
                for (int ch = 0; ch < channels; ++ch)
                {
                    const double L = units[(size_t) ch].timing.process (det[ch] * det[ch]);
                    over[ch] = L * lnToDb + levelOffset - threshold;

                    if (slaveAlone)
                    {
                        // Control voltage comes from an empty strap jack: unity gain.
                        gr[ch] = 0.0;
                        gain[ch] = 1.0;
                    }
                    else
                    {
                        gr[ch] = slope * knee (over[ch]);
                        gain[ch] = dbToGain (gainDb - gr[ch]);
                    }
                }
            }

            for (int ch = 0; ch < channels; ++ch)
            {
                auto& u = units[(size_t) ch];
                const double out = u.audioHp.process (in[ch]) * gain[ch];

                const double metered = u.meterHp.process (panel.displayInput ? in[ch] : out);
                meterMs += u.meter.process (metered * metered);

                // Hard-wire bypass: input jack straight to output jack.
                data[ch][n] = panel.bypass ? (Sample) in[ch] : (Sample) out;

                maxOver = std::max (maxOver, over[ch]);
                maxGr = std::max (maxGr, gr[ch]);
            }
        }

        if (numSamples > 0)
        {
            const double ms = meterMs / (double) (numSamples * channels);
            meters.levelDb.store ((float) (calibration.meanSquareToDbu (ms) - 4.0), std::memory_order_relaxed);
            meters.gainReductionDb.store ((float) maxGr, std::memory_order_relaxed);
            meters.overThresholdDb.store ((float) maxOver, std::memory_order_relaxed);
        }
    }

    /** Settled gain reduction for a steady level, for tests and displays. */
    double staticGainReductionDb (double levelDbu) const
    {
        return PanelLaws::compressionSlope (panel.ratio)
             * knee (levelDbu - PanelLaws::thresholdDbu (panel.threshold));
    }

    MeterState meters;

    /** OverEasy amber LED turns on this far below threshold: the U10 comparator
        reference is -15 V * R102 / (R101 + R102) against TP2 at 180 mV/dB. */
    static double overEasyAmberOnsetDb() { return -(15.0 * 6.04 / (56.2 + 6.04)) / 0.180; }

private:
    double knee (double overDb) const
    {
        return panel.overEasy ? kneeStage.overEasy (overDb) : KneeStage::hard (overDb);
    }

    struct Unit
    {
        CouplingHighPass audioHp, detectorHp, meterHp;
        TimingNode timing;
        MeterDetector meter;
    };

    std::array<Unit, maxChannels> units;
    TimingNode linkedTiming;
    KneeStage kneeStage;
    Calibration calibration;
    PanelState panel;

    double fs = 48000.0;
    int channels = 1;
    double smoothing = 1.0;
    bool haveTargets = false;
    int meterDecimator = 0;

    double targetThreshold = 0.0, targetSlope = 0.0, targetGain = 0.0;
    double threshold = 0.0, slope = 0.0, gainDb = 0.0;
};
} // namespace jb160
