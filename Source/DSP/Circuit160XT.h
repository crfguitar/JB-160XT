// JB 160XT - circuit-derived building blocks (no JUCE dependency).
//
// Everything here is taken from the 160XT schematic (drawing 160XT-00-01-02-01)
// and the factory test procedure, Rev 02. Component designators are quoted so a
// value can be checked against the scan. Where a value comes from a
// specification instead of a component, that is stated.
#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace jb160
{
constexpr double kPi = 3.14159265358979323846;
constexpr double kLn10 = 2.302585092994046;

// 1N4148-class small-signal diode (CR8-CR12, CR5-CR7), standard SPICE values.
constexpr double kDiodeIs = 2.52e-9;
constexpr double kDiodeN = 1.752;
constexpr double kVt = 0.02585;

inline double dbToGain (double db) { return std::exp (db * (kLn10 / 20.0)); }

//==============================================================================
/** Level calibration between the digital and analog domains.

    The hardware works in dBu ("dBm" on the panel). A digital full-scale sine
    (sample peak 1.0) is taken to be +22 dBu, so -18 dBFS sits at the unit's
    nominal +4 dBu and lights the "0" meter LED. This is a convention, not a
    measurement of any interface.
*/
struct Calibration
{
    double fullScaleSineDbu = 22.0;

    /** dBu of a signal whose mean square (digital units) is ms. */
    double meanSquareToDbu (double ms) const
    {
        return 10.0 * std::log10 (std::max (ms, 1.0e-30)) + 3.0103 + fullScaleSineDbu;
    }
};

//==============================================================================
/** Front-panel pot laws. Rotation is 0 (fully CCW) to 1 (fully CW). */
struct PanelLaws
{
    /** THRESHOLD, R68 20k linear between +/-15 V, summed through R65 64k9 into
        U5 (feedback R67 23k2), 180 mV/dB. The 64k9 load bends the law slightly. */
    static double thresholdDbu (double rot)
    {
        const double wiper = 15.0 * (2.0 * rot - 1.0) / (1.0 + (20.0 / 64.9) * rot * (1.0 - rot));
        return -10.0 + wiper * (23.2 / 64.9) / 0.180;
    }

    /** OUTPUT GAIN, R86 20k linear between +/-15 V through R87 787k into U7
        (feedback R90 113k), 100 mV/dB at the U7 output. */
    static double outputGainDb (double rot)
    {
        return 15.0 * (2.0 * rot - 1.0) * (113.0 / 787.0) / 0.100;
    }

    /** COMPRESSION RATIO, R79 20k linear from the -145 mV/dB bus to ground with
        R80 8k25 up and R81 4k32 down from the wiper, loaded by R89 80k6.
        Returns the compression slope c: gain reduction in dB per dB over
        threshold. 0 = 1:1, 1 = infinity:1, 2 = -1:1.

        R82 (INF trim) is modelled as a conductance to ground chosen so that the
        slope is exactly 1 with the knob on the printed infinity mark, which is
        what factory step H sets. */
    static double compressionSlope (double rot)
    {
        rot = std::clamp (rot, 0.0, 1.0);
        if (rot <= 0.0) return 0.0;
        if (rot >= 1.0) return 2.0;

        const double gUp = 1.0 / (20.0 * (1.0 - rot)) + 1.0 / 8.25;
        const double gDown = 1.0 / (20.0 * rot) + 1.0 / 4.32 + 1.0 / 80.6 + infinityTrimConductance();
        return 2.0 * gUp / (gUp + gDown);
    }

    /** Rotation of the printed infinity:1 mark (+75 degrees of a 300 degree sweep). */
    static constexpr double infinityMarkRotation = 0.75;

    static double infinityTrimConductance()
    {
        const double r = infinityMarkRotation;
        const double gUp = 1.0 / (20.0 * (1.0 - r)) + 1.0 / 8.25;
        const double gDownFixed = 1.0 / (20.0 * r) + 1.0 / 4.32 + 1.0 / 80.6;
        return std::max (0.0, gUp - gDownFixed); // slope 1 <=> gUp == gDown
    }

    /** Ratio as printed on the panel (in:out). Infinity is returned as +inf. */
    static double ratioFromSlope (double c)
    {
        const double outSlope = 1.0 - c;
        if (std::abs (outSlope) < 1.0e-4) return INFINITY;
        return 1.0 / outSlope;
    }
};

//==============================================================================
/** Threshold stage: hard knee and OverEasy.

    Input is level over threshold in dB (TP2 is 180 mV/dB about 0 V at
    threshold). Output is the "drive" in dB that the ratio pot then scales, i.e.
    the gain reduction the unit would apply at infinity:1.

    Hard knee is the precision rectifier around U5 (R74 10k, R77 8k06, CR11/CR12
    inside the loop): exactly max(0, over).

    OverEasy is the U6 path: non-inverting amp (R69 11k5 / R70 604R divider,
    gain 1 + R73/R72) feeding CR10 open-loop into R75 10k and the U6 inverter
    (R76 121k). CR5-CR7 with R71/R72 add one diode drop of forward bias. The
    diode's exponential turn-on is the OverEasy curve.
*/
class KneeStage
{
public:
    KneeStage() { buildTable(); }

    static double hard (double overDb) { return std::max (0.0, overDb); }

    double overEasy (double overDb) const
    {
        const double pos = (overDb - tableStartDb) / tableStepDb;
        if (pos <= 0.0) return 0.0;

        const auto last = (double) (table.size() - 1);
        if (pos >= last)
            return table.back() + (overDb - (tableStartDb + last * tableStepDb)); // unity slope beyond

        const auto i = (size_t) pos;
        const double f = pos - (double) i;
        return table[i] + f * (table[i + 1] - table[i]);
    }

    /** Direct circuit solution (used to build the table and by the tests). */
    static double solveOverEasy (double overDb)
    {
        const double nVt = kDiodeN * kVt;
        const double tp2 = 0.180 * overDb;
        const double vPlus = tp2 * 604.0 / (11500.0 + 604.0);

        // Bias string: R71 191k from -15 V, three diodes to ground, R72 30k1 to
        // the inverting input (which sits at vPlus).
        double vd = 0.45;
        for (int i = 0; i < 8; ++i)
        {
            const double ib = (15.0 - 3.0 * vd) / 191.0e3 - (vPlus + 3.0 * vd) / 30.1e3;
            vd = nVt * std::log (std::max (ib, 1.0e-9) / kDiodeIs + 1.0);
        }

        const double drive = vPlus * (1.0 + 10.0 / 30.1) + 3.0 * vd * (10.0 / 30.1);

        // CR10 in series with R75 into the U6b virtual ground:
        //   drive = nVt ln(i/Is + 1) + i R75
        const double r75 = 10.0e3;
        double i = std::max (0.0, (drive - 0.45) / r75) + 1.0e-9;
        for (int it = 0; it < 60; ++it)
        {
            const double f = nVt * std::log (i / kDiodeIs + 1.0) + i * r75 - drive;
            const double df = nVt / (i + kDiodeIs) + r75;
            double next = i - f / df;
            if (next <= 0.0) next = i * 0.1;
            if (std::abs (next - i) < 1.0e-15) { i = next; break; }
            i = next;
        }

        // U6b: -i * R76; hard-knee bus scale is 180 mV/dB * 8.06/10.
        const double busVoltsPerDb = 0.180 * 0.806;
        return std::clamp (i * 121.0e3 / busVoltsPerDb, 0.0, 90.0);
    }

private:
    void buildTable()
    {
        const int n = (int) std::lround ((tableEndDb - tableStartDb) / tableStepDb) + 1;
        table.resize ((size_t) n);
        for (int i = 0; i < n; ++i)
            table[(size_t) i] = solveOverEasy (tableStartDb + i * tableStepDb);
    }

    static constexpr double tableStartDb = -70.0;
    static constexpr double tableEndDb = 80.0;
    static constexpr double tableStepDb = 0.05;
    std::vector<double> table;
};

//==============================================================================
/** One-pole high-pass for the coupling networks (C16/R8 audio, C37/R42 detector,
    C61/R111 meter). */
class CouplingHighPass
{
public:
    void prepare (double sampleRate, double cornerHz)
    {
        a = std::exp (-2.0 * kPi * cornerHz / sampleRate);
        reset();
    }

    void reset() { x1 = y1 = 0.0; }

    double process (double x)
    {
        const double y = a * (y1 + x - x1);
        x1 = x;
        y1 = y + 1.0e-30;
        return y;
    }

private:
    double a = 0.0, x1 = 0.0, y1 = 0.0;
};

//==============================================================================
/** RMS detector timing node: 146742 (2252-family) log-domain filter with the
    non-linear capacitor built around U8.

    The detector injects a current I_T (u e^-L - 1) into the timing node, where
    u is the squared input and L the node voltage in units of V_T (so e^L is the
    running mean square). The node is loaded by
        C41 (10 uF)  to the U8 virtual ground,
        C47 (220 nF) to the U8 output,
    and U8 has C45 (47 nF), R60 (8M2) and CR8/CR9 (anti-parallel) as feedback.

    For small, slow changes U8 multiplies C47 (Miller effect) and the node is
    slow, which keeps low-frequency ripple - and so distortion - down. Once the
    U8 output reaches a diode drop the multiplication stops and the node is
    fast. That is the 160XT's program-dependent attack and release.

    State: L and a = U8 output / V_T. Solved per sample with backward Euler and
    Newton iteration; the Jacobian determinant is positive everywhere, so the
    step has a unique solution.
*/
class TimingNode
{
public:
    void prepare (double sampleRate)
    {
        h = 1.0 / sampleRate;

        // Fast time constant: the specification's 125 dB/s release rate, which
        // for a log-RMS detector is 4.343 / tau. The parts (C41 + C47 = 10.22 uF,
        // R59 2M0 timing resistor) give 32-35 ms depending on temperature and
        // the +/-20 % electrolytic, so the spec figure is used to place it.
        const double tauFast = 4.342944819 / 125.0;
        beta = cFast / tauFast;

        reset (1.0e-12);
    }

    void reset (double meanSquare)
    {
        L = std::log (std::max (meanSquare, floorU));
        a = 0.0;
    }

    /** u: instantaneous squared input. Returns ln(mean square). */
    double process (double u)
    {
        u = std::max (u, floorU);

        const double L0 = L, a0 = a;
        const double invH = 1.0 / h;

        for (int it = 0; it < 16; ++it)
        {
            const double e = u * std::exp (-L);
            const double ex = std::exp (std::clamp (a / kDiodeN, -45.0, 45.0));
            const double sh = 0.5 * (ex - 1.0 / ex);
            const double ch = 0.5 * (ex + 1.0 / ex);
            const double G = a * gLeak + gDiode * sh;
            const double dG = gLeak + gDiode * ch / kDiodeN;

            const double f1 = cTotal * (L - L0) * invH - beta * (e - 1.0) + kMiller * G;
            const double f2 = c6 * (a - a0) * invH + c3 * (L - L0) * invH + G;

            const double j11 = cTotal * invH + beta * e;
            const double j12 = kMiller * dG;
            const double j21 = c3 * invH;
            const double j22 = c6 * invH + dG;
            const double det = j11 * j22 - j12 * j21;

            double dL = (f1 * j22 - f2 * j12) / det;
            double da = (f2 * j11 - f1 * j21) / det;

            dL = std::clamp (dL, -3.0, 3.0);
            da = std::clamp (da, -6.0, 6.0);

            L -= dL;
            a -= da;

            if (std::abs (dL) < 1.0e-10 && std::abs (da) < 1.0e-8)
                break;
        }

        if (! std::isfinite (L) || ! std::isfinite (a))
        {
            L = std::log (u);
            a = 0.0;
        }

        return L;
    }

    double getLogMeanSquare() const { return L; }

    // Capacitances in uF, conductances in uS (i.e. currents are in units of
    // V_T * uA/V), which keeps the Newton system well scaled.
    static constexpr double c3 = 10.0;    // C41
    static constexpr double c5 = 0.22;    // C47
    static constexpr double c6 = 0.047;   // C45
    static constexpr double kMiller = c5 / c6;
    static constexpr double cFast = c3 + c5;
    static constexpr double cTotal = c3 + c5 + c5 * c3 / c6;
    static constexpr double gLeak = 1.0 / 8.2;                    // R60 8M2
    static constexpr double gDiode = 2.0 * kDiodeIs / kVt * 1.0e6; // CR8/CR9

private:
    static constexpr double floorU = 1.0e-13; // about -105 dBu; R46 22M keeps the real part alive similarly

    double h = 1.0 / 48000.0;
    double beta = 0.0;
    double L = -28.0, a = 0.0;
};

//==============================================================================
/** Plain 2252 log-RMS averager (no non-linear capacitor), used by the level
    meter's own detector U14 (C64 10 uF, R115 2M2 -> about 35 ms). */
class MeterDetector
{
public:
    void prepare (double sampleRate)
    {
        coeff = 1.0 - std::exp (-1.0 / (0.035 * sampleRate));
        ms = 1.0e-12;
    }

    void reset() { ms = 1.0e-12; }

    double process (double u)
    {
        ms += coeff * (u - ms);
        ms = std::max (ms, 1.0e-13);
        return ms;
    }

    double getMeanSquare() const { return ms; }

private:
    double coeff = 0.0, ms = 1.0e-12;
};
} // namespace jb160
