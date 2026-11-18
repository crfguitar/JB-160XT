# Engineering findings and evidence boundaries

Research date: 2026-11-15. Source IDs refer to [SOURCE_INDEX.md](SOURCE_INDEX.md). PDF page numbers below are one-based file pages, which can differ from printed numbers.

## The target is specifically the 160XT

The hardware target is the user's own 160XT. The original 160/VU, 160X, XT and later 160A belong to a family, but matching one does not establish a match to another. The manufacturer describes XT changes including balanced output, XLR connectors and true power summing. [HW01, PDF p4](../01_Hardware_Service/dbx_160X_XT_Service_Manual.pdf)

Overloud Comp160 explicitly offers VU, XT and A models. Its product page says the reference XT had an optional transformer. Treat its XT sound as one comparison specimen, not a universal target for every stock XT. [WEB01](Source_Pages/Overloud_Comp160.txt)

Waves dbx160, UA dbx160 and NI VC160 are useful original-160-family comparators and UI references. Their material does not establish them as XT emulations. Keep their custom sidechain processing, noise modes, parallel mixing and other extensions separate from hardware-derived behavior. [PL01, PL03, WEB04]

## Documented operating envelope

These are specification anchors, not measured values from the user's unit. HW01 PDF p25 is the compact source table.

| Property | Manufacturer's stated value | Development use |
| --- | --- | --- |
| Threshold | -40 to +20 dBu | Establish analog-to-digital level mapping |
| Output gain | -20 to +20 dB | Reproduce scale and gain placement |
| Ratio | 1:1 through infinity:1 to -1:1 | Include negative output slope and a continuous control around infinity |
| Compression | More than 60 dB maximum | Preserve numerical range and low-level behavior |
| Knee | Selectable hard knee / OverEasy | Fit both transfer curves |
| Frequency response | 20 Hz-20 kHz, +/-0.5 dB | A specification band, not an exact EQ trace |
| THD / SMPTE IMD | Below 0.2% as specified | Context-dependent performance limits, not a saturation setting |
| Maximum main input/output | +24 dBu; output specified into 600 ohms | Model headroom only after converter calibration |
| Maximum detector input | +28 dBu | Separate detector range from main audio input |
| Meter nominal reference | 0 indication at +4 dBu | Do not confuse analog meter zero with digital full scale |

The factory procedure uses some different test limits from the specification table. Keep the conditions attached: for example, frequency response acceptance of +0/-1 dB relative to 1 kHz, and separate THD tests at ordinary and maximum output levels. These do not provide an exact frequency-response curve. [HW01 pp30-33]

## Timing deserves priority over added coloration

The XT specification describes program-dependent attack: 15 ms for 10 dB, 5 ms for 20 dB, and 3 ms for 30 dB. Release examples are 8 ms for 1 dB, 80 ms for 10 dB and 400 ms for 50 dB, with a stated 125 dB/s rate. These examples do not define the settling convention, residual signal, ratio or a complete time-domain model. Do not convert them directly into three fixed attack presets. [HW01 p25]

The 2252 documentation explains log-domain averaging, linked attack/release, tone-burst response and detector ripple. Its timing expression is `tau = C_T * V_T / I_T`. Device behavior, surrounding circuit values and compression ratio all matter. A generic peak detector with independent exponential attack/release is only a comparison baseline. The exact XT dynamics still need to be traced and measured. [IC02 pp3-5; DSP02; DSP03]

Practical inference: measure amplitude steps at several frequencies, ratios and histories before choosing the detector algorithm. Low-frequency gain ripple, overshoot and recovery shape can be more diagnostic than matching one nominal attack time.

## OverEasy is more specific than a generic soft knee

The factory procedure provides useful static anchors with the threshold at zero and the ratio at infinity. In OverEasy mode, its p32 test expects approximately -6 dB output for zero-level input, with the stated tolerance; high input approaches the threshold-limited output. Its p31 procedure also tests the threshold LEDs through the knee region. Read the scans for the exact level notation and conditions. [HW01 pp31-32]

These anchors are valuable checks against a fitted knee model. They are insufficient to establish the complete curve or a universal knee width. A conventional quadratic soft knee should remain a provisional implementation until measurements and the analog control circuit support it.

For a mathematical baseline, if `x` and `T` share the same calibrated dB reference, a hard-knee, steady-state compressor above threshold has `y = T + (x-T)/R + M`. Infinity gives zero output slope; negative ratios give negative output slope. This baseline omits timing, analog error and OverEasy curvature. [DSP01; DN01A; HW01 p10]

An implementation can use inverse slope `s = 1/R`, passing continuously through zero at infinity, rather than trying to interpolate a numerically infinite ratio. This is a development suggestion, not a claim about the original potentiometer circuit.

## Three different meanings of response curves

1. **Main audio response:** magnitude/phase through the input, gain cell and output at a stated level, gain and load.
2. **Detector response:** the frequency-dependent sensitivity of compression, which can differ from audio response.
3. **Transfer curve:** settled output level versus input level for threshold, knee and ratio settings.

The XT has no front-panel audio EQ. Its manual shows frequency-weighted compression using an external equalizer in the detector path. That changes what drives broadband gain reduction; it does not turn the unit into a multiband equalizer. A plugin's LF-sensitivity extension is not evidence of a stock XT audio-path bass shelf. [HW01 pp13-15; WEB01]

No calibrated public XT frequency/phase trace with enough unit, load, level and settings information was verified during this search. The saved graphs are manufacturer transfer illustrations, component measurements and detector plots, all labeled in the curve guide. Do not digitize a VCA datasheet graph and call it a measured XT response.

## VCA and component evidence

The legacy schematic set labels a VCA and RMS blocks using dbx internal part numbering. The 2150-family VCA and 2252-family detector documentation provide relevant theory. The physical unit's chip markings and PCB revision still need confirmation before selecting a component-level model. A later replacement or modification can change what is installed.

The 2150 datasheet's p5 gain equation uses approximately 6.1 mV/dB at the stated temperature. Later 218x references use different control scaling and revised performance. Do not silently replace legacy data with successor data. The factory's approximately 180 mV/dB tracking test is at an amplified circuit test point, not the bare IC control constant. [IC01 p5; IC03; IC04; HW01 p30]

Distortion varies with level, gain and symmetry adjustment. The archived plots, dbx AN103, THAT design notes and Hebert's VCA paper are useful for separating gain-cell distortion, control feedthrough, noise modulation and output-stage effects. A single generic waveshaper cannot be assumed to capture all of these. [IC01 pp5-7; AN103; DN106; DN110; DSP06]

## Linking, metering and connection details

True power summing is documented for XT stereo strapping. A stereo digital design should be checked with equal-level correlated, uncorrelated and polarity-inverted signals; summing raw L+R before detection can cancel anti-phase signals. Whether to sum or average power for a particular digital calibration is a measurement question, not an assumption. [HW01 pp4,11,17; DN116]

The factory procedure describes XLR pin 3 as positive and pin 2 as negative. Modern interface wiring may use the opposite convention. Record actual polarity when aligning round-trip captures. The exact connector path, output load and optional transformer configuration belong in capture metadata. [HW01 p32]

Meter counts differ across supplied documents: the service text lists a 12-segment gain-reduction display, while another XT manual specification lists 13. Confirm the user's panel and schematic rather than copying a plugin picture. Input/output meter calibration does not change gain-reduction meter calibration. [HW01 pp5,11; existing owner-manual scans]

## Best route from research to a faithful recreation

First document the hardware variant and calibrate a line-level interface loopback. Then obtain static transfer, timing, linear response and nonlinear spectra from the same unit. Fit an interpretable detector/gain model and add only the nonidealities needed to explain repeatable residual errors. Use independent musical captures to test the result.

The general DSP papers offer options and evaluation methods, not an exact XT algorithm. Following the research pass, the user explicitly selected a JUCE/C++ build and a pixel-for-pixel hardware UI target. The DAW and interface remain unspecified. See [instructions.md](../instructions.md) for coding-assistant guidance and the [measurement plan](../07_Hardware_Measurement_Plan/MEASUREMENT_PLAN.md) for hardware characterization.
