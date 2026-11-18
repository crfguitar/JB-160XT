// JB 160XT - DSP checks against the factory test procedure and the published
// specification. Console program; returns non-zero if any check fails.

#include "../Source/DSP/Compressor160XT.h"

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

using namespace jb160;

static int failures = 0;

static void check (const std::string& name, double value, double lo, double hi, const char* unit = "")
{
    const bool ok = value >= lo && value <= hi;
    if (! ok) ++failures;
    std::printf ("  [%s] %-58s %9.3f %s  (allowed %.3f .. %.3f)\n", ok ? "ok  " : "FAIL", name.c_str(), value, unit, lo, hi);
}

static double invert (const std::function<double (double)>& f, double target)
{
    double lo = 0.0, hi = 1.0;
    for (int i = 0; i < 60; ++i)
    {
        const double mid = 0.5 * (lo + hi);
        (f (mid) < target ? lo : hi) = mid;
    }
    return 0.5 * (lo + hi);
}

static double rotForThreshold (double dbu) { return invert (PanelLaws::thresholdDbu, dbu); }
static double rotForGain (double db) { return invert (PanelLaws::outputGainDb, db); }
static double rotFromDegrees (double deg) { return (deg + 150.0) / 300.0; }

struct Rig
{
    double fs = 48000.0;
    Calibration cal;
    Compressor160XT comp;
    double phase = 0.0;

    Rig (double sampleRate, const PanelState& p) : fs (sampleRate)
    {
        comp.prepare (fs, 1);
        comp.setCalibration (cal);
        comp.setPanel (p);
    }

    static double amplitudeForDbu (double dbu, const Calibration& c) { return std::pow (10.0, (dbu - c.fullScaleSineDbu) / 20.0); }

    /** Runs a sine burst, returns the output samples. */
    std::vector<double> run (double dbu, double freq, double seconds)
    {
        const int n = (int) (seconds * fs);
        std::vector<double> buf ((size_t) n);
        const double amp = amplitudeForDbu (dbu, cal);
        const double inc = 2.0 * kPi * freq / fs;

        for (int i = 0; i < n; ++i)
        {
            buf[(size_t) i] = amp * std::sin (phase);
            phase += inc;
        }

        double* chans[1] = { buf.data() };
        const int block = 256;
        for (int pos = 0; pos < n; pos += block)
        {
            double* c[1] = { chans[0] + pos };
            comp.process (c, std::min (block, n - pos));
        }

        return buf;
    }

    double rmsDbu (const std::vector<double>& x, double lastSeconds) const
    {
        const size_t n = std::min (x.size(), (size_t) (lastSeconds * fs));
        double sum = 0.0;
        for (size_t i = x.size() - n; i < x.size(); ++i) sum += x[i] * x[i];
        return cal.meanSquareToDbu (sum / (double) n);
    }

    double settledOutputDbu (double inDbu, double freq = 1000.0)
    {
        auto y = run (inDbu, freq, 3.0);
        return rmsDbu (y, 0.5);
    }
};

/** Envelope of a sine-carrier output in dB, by RMS over whole carrier cycles. */
static std::vector<double> envelopeDb (const std::vector<double>& x, double fs, double freq, const Calibration& cal)
{
    const int win = (int) std::lround (fs / freq);
    std::vector<double> env;
    for (size_t i = 0; i + (size_t) win <= x.size(); i += (size_t) win)
    {
        double s = 0.0;
        for (int k = 0; k < win; ++k) s += x[i + (size_t) k] * x[i + (size_t) k];
        env.push_back (cal.meanSquareToDbu (s / win));
    }
    return env;
}

int main()
{
    std::printf ("JB 160XT DSP checks\n\n== Panel laws ==\n");

    check ("threshold, fully CCW", PanelLaws::thresholdDbu (0.0), -41.0, -39.0, "dBu");
    check ("threshold, centre (-10 mark)", PanelLaws::thresholdDbu (0.5), -10.5, -9.5, "dBu");
    check ("threshold, 0 mark (+50 deg)", PanelLaws::thresholdDbu (rotFromDegrees (50)), -1.5, 1.5, "dBu");
    check ("threshold, fully CW", PanelLaws::thresholdDbu (1.0), 19.0, 21.0, "dBu");
    check ("output gain, fully CW (spec +20, test +/-3)", PanelLaws::outputGainDb (1.0), 17.0, 23.0, "dB");
    check ("output gain, centre", PanelLaws::outputGainDb (0.5), -0.01, 0.01, "dB");

    check ("ratio slope, 1:1 end", PanelLaws::compressionSlope (0.0), 0.0, 0.0);
    check ("ratio slope, infinity mark", PanelLaws::compressionSlope (0.75), 0.9999, 1.0001);
    check ("ratio slope, -1:1 end", PanelLaws::compressionSlope (1.0), 2.0, 2.0);

    std::printf ("\n  Ratio at the printed marks (model vs legend):\n");
    const double markDeg[] = { -150, -100, -55, -24, 18, 44, 75, 103, 126, 150 };
    const char* markName[] = { "1:1", "2:1", "3:1", "4:1", "6:1", "10:1", "inf:1", "-5:1", "-2:1", "-1:1" };
    for (int i = 0; i < 10; ++i)
    {
        const double c = PanelLaws::compressionSlope (rotFromDegrees (markDeg[i]));
        const double r = PanelLaws::ratioFromSlope (c);
        std::printf ("    %-6s at %+5.0f deg -> slope %.3f, ratio %s%.2f:1\n", markName[i], markDeg[i], c, std::isinf (r) ? "inf " : "", std::isinf (r) ? 0.0 : r);
    }

    std::printf ("\n== OverEasy curve (drive in dB at infinity:1) ==\n");
    for (double over : { -30.0, -20.0, -10.0, -5.0, 0.0, 5.0, 10.0, 20.0, 30.0, 40.0 })
        std::printf ("    %+5.0f dB over threshold -> %6.2f dB reduction, output %+6.2f\n", over, KneeStage::solveOverEasy (over), over - KneeStage::solveOverEasy (over));

    check ("OverEasy drive well below threshold", KneeStage::solveOverEasy (-30.0), 0.0, 0.05, "dB");

    {
        KneeStage k;
        double worst = 0.0;
        for (double o = -60.0; o < 70.0; o += 0.0137) worst = std::max (worst, std::abs (k.overEasy (o) - KneeStage::solveOverEasy (o)));
        check ("OverEasy table interpolation error", worst, 0.0, 0.002, "dB");
    }

    std::printf ("\n== Factory procedure, section H (threshold 0, infinity:1, 1 kHz) ==\n");
    PanelState p;
    p.threshold = rotForThreshold (0.0);
    p.ratio = 0.75;
    p.outputGain = rotForGain (0.0);

    { Rig r (48000, p); check ("hard knee, +10 dBm in -> 0 dBm out (+/-0.5)", r.settledOutputDbu (10.0), -0.5, 0.5, "dBu"); }
    { Rig r (48000, p); check ("hard knee, +20 dBm in -> 0 dBm out (+/-1.5)", r.settledOutputDbu (20.0), -1.5, 1.5, "dBu"); }
    { Rig r (48000, p); check ("hard knee,   0 dBm in -> 0 dBm out (+/-1.0)", r.settledOutputDbu (0.0), -1.0, 1.0, "dBu"); }
    { Rig r (48000, p); check ("hard knee, -10 dBm in passes at unity", r.settledOutputDbu (-10.0), -10.2, -9.8, "dBu"); }

    p.overEasy = true;
    { Rig r (48000, p); check ("OverEasy,   0 dBm in -> -6 dBm out (+/-2)", r.settledOutputDbu (0.0), -8.0, -4.0, "dBu"); }
    { Rig r (48000, p); check ("OverEasy, +20 dBm in -> 0 dBm out (+/-1.5)", r.settledOutputDbu (20.0), -1.5, 1.5, "dBu"); }
    check ("OverEasy amber LED onset (-10 dBm +/-3)", Compressor160XT::overEasyAmberOnsetDb(), -13.0, -7.0, "dB");
    p.overEasy = false;

    {
        PanelState q = p;
        q.threshold = rotForThreshold (-20.0);
        Rig r (48000, q);
        check ("hard knee, threshold -20, 0 dBm in -> -20 (+/-3)", r.settledOutputDbu (0.0), -23.0, -17.0, "dBu");
    }
    {
        PanelState q = p;
        q.ratio = 0.0;
        q.outputGain = 1.0;
        Rig r (48000, q);
        check ("1:1, gain fully CW, 0 dBm in -> +20 (+/-3)", r.settledOutputDbu (0.0), 17.0, 23.0, "dBu");
    }
    {
        PanelState q = p;
        q.ratio = 1.0; // -1:1
        Rig r (48000, q);
        check ("-1:1, +10 dBm in -> -10 dBm out", r.settledOutputDbu (10.0), -10.6, -9.4, "dBu");
    }
    {
        PanelState q = p;
        q.slave = true; // nothing on the strap jack
        q.outputGain = 1.0;
        Rig r (48000, q);
        check ("SLAVE with no master: unity gain", r.settledOutputDbu (10.0), 9.9, 10.1, "dBu");
    }
    {
        PanelState q = p;
        q.bypass = true;
        Rig r (48000, q);
        check ("BYPASS: hard-wire", r.settledOutputDbu (10.0), 9.999, 10.001, "dBu");
    }

    std::printf ("\n== Timing (spec: attack 15 ms/10 dB, 5 ms/20 dB, 3 ms/30 dB; release 125 dB/s) ==\n");
    for (double fs : { 44100.0, 48000.0, 96000.0 })
    {
        std::printf ("  sample rate %.0f\n", fs);
        const double carrier = 3000.0;
        for (double step : { 10.0, 20.0, 30.0 })
        {
            PanelState q = p;
            q.threshold = rotForThreshold (-20.0);
            Rig r (fs, q);
            r.run (-20.0, carrier, 2.0);                 // settle at threshold
            auto y = r.run (-20.0 + step, carrier, 0.5); // step up
            auto env = envelopeDb (y, fs, carrier, r.cal);
            // gain reduction reaches 63 % of its final value
            const double target = (-20.0 + step) - 0.632 * step;
            double t = -1.0;
            for (size_t i = 0; i < env.size(); ++i)
                if (env[i] <= target) { t = (double) (i + 1) / carrier; break; }

            const double nominal = step == 10.0 ? 15.0 : step == 20.0 ? 5.0 : 3.0;
            char name[96];
            std::snprintf (name, sizeof name, "attack for a %.0f dB step (63%% of reduction)", step);
            check (name, t * 1000.0, nominal * 0.6, nominal * 1.6, "ms");
        }

        {
            PanelState q = p;
            q.threshold = rotForThreshold (-30.0);
            Rig r (fs, q);
            r.run (10.0, carrier, 2.0);            // 40 dB of reduction
            auto y = r.run (-30.0, carrier, 0.6);  // drop to threshold
            auto env = envelopeDb (y, fs, carrier, r.cal);
            // output climbs from -70 back to -30; measure the slope between 35 and 10 dB of reduction remaining
            double t1 = -1, t2 = -1;
            for (size_t i = 0; i < env.size(); ++i)
            {
                const double gr = -30.0 - env[i];
                if (t1 < 0 && gr <= 35.0) t1 = (double) i / carrier;
                if (t2 < 0 && gr <= 10.0) { t2 = (double) i / carrier; break; }
            }
            check ("release rate, large recovery", 25.0 / (t2 - t1), 100.0, 150.0, "dB/s");
        }
    }

    std::printf ("\n== Non-linear capacitor: ripple and low-frequency distortion ==\n");
    for (double f : { 50.0, 100.0, 1000.0 })
    {
        PanelState q = p;
        q.threshold = rotForThreshold (-10.0);
        Rig r (96000, q);
        r.run (0.0, f, 3.0);
        auto y = r.run (0.0, f, 1.0);
        // third-harmonic level relative to fundamental by single-bin DFT
        auto bin = [&] (double freq)
        {
            double re = 0, im = 0;
            for (size_t i = 0; i < y.size(); ++i)
            {
                const double ph = 2.0 * kPi * freq * (double) i / 96000.0;
                re += y[i] * std::cos (ph);
                im += y[i] * std::sin (ph);
            }
            return std::sqrt (re * re + im * im);
        };
        const double thd3 = 100.0 * bin (3.0 * f) / bin (f);
        std::printf ("    %6.0f Hz, 10 dB of limiting: 3rd harmonic %.3f %%\n", f, thd3);
        if (f == 1000.0) check ("THD at 1 kHz under compression (spec < 0.2 %)", thd3, 0.0, 0.2, "%");
    }

    std::printf ("\n== Stereo strapping (true power summing) ==\n");
    {
        PanelState q = p;
        q.slave = true;
        Compressor160XT c;
        Calibration cal;
        c.prepare (48000, 2);
        c.setPanel (q);
        const int n = 48000 * 3;
        std::vector<double> l ((size_t) n), rr ((size_t) n);
        const double amp = Rig::amplitudeForDbu (10.0, cal);
        for (int i = 0; i < n; ++i)
        {
            l[(size_t) i] = amp * std::sin (2.0 * kPi * 1000.0 * i / 48000.0);
            rr[(size_t) i] = -l[(size_t) i]; // anti-phase: a voltage sum would cancel
        }
        double* chans[2] = { l.data(), rr.data() };
        c.process (chans, n);
        double s = 0;
        for (int i = n - 24000; i < n; ++i) s += l[(size_t) i] * l[(size_t) i];
        check ("anti-phase pair still limited (each +10 in -> -3 out)", cal.meanSquareToDbu (s / 24000.0), -3.6, -2.4, "dBu");
    }

    std::printf ("\n== External detector (sidechain) ==\n");
    {
        PanelState q = p;
        q.externalDetector = true;
        Compressor160XT c;
        Calibration cal;
        c.prepare (48000, 1);
        c.setPanel (q);
        const int n = 48000 * 2;
        std::vector<double> bass ((size_t) n), key ((size_t) n);
        for (int i = 0; i < n; ++i)
        {
            bass[(size_t) i] = Rig::amplitudeForDbu (10.0, cal) * std::sin (2.0 * kPi * 55.0 * i / 48000.0);
            // key: silent for the first second, +10 dBu for the second
            key[(size_t) i] = i < 48000 ? 0.0 : Rig::amplitudeForDbu (10.0, cal) * std::sin (2.0 * kPi * 1000.0 * i / 48000.0);
        }
        double* chans[1] = { bass.data() };
        const double* det[1] = { key.data() };
        c.process (chans, det, 1, n);
        auto level = [&] (int from, int to)
        {
            double sum = 0;
            for (int i = from; i < to; ++i) sum += bass[(size_t) i] * bass[(size_t) i];
            return cal.meanSquareToDbu (sum / (to - from));
        };
        check ("loud bass, silent key: bass untouched", level (24000, 48000), 9.8, 10.1, "dBu");
        check ("key at +10: bass ducked by 10 dB", level (72000, 96000), -0.5, 0.5, "dBu");
    }

    std::printf ("\n%s (%d failure%s)\n", failures == 0 ? "ALL CHECKS PASSED" : "CHECKS FAILED", failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
