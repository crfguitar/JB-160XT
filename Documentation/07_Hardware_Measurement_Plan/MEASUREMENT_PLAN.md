# Hardware characterization and DAW validation plan

This is proposed future work. No test audio has been sent, no hardware has been opened or adjusted, and no DAW session has been changed. The physical target is one user-owned 160XT; the DAW, audio interface and exact hardware revision are not yet recorded.

## 1. Establish the specimen and the capture chain

Record serial number, panel version, knob positions, optional output transformer, service/modification history, and the connector path used. When its internals are appropriately documented, record PCB revision and actual VCA/RMS markings. A stock electronic output and optional transformer output should have distinct dataset labels.

Use a line-output/line-input route: DAW test playback -> interface DAC -> hardware input -> hardware output -> interface ADC -> raw capture track. Capture a dry reference or repeatable loopback as well. Do not monitor the capture back into its own stimulus path. Leave master effects, normalization, limiting and automatic gain disabled. Do not overwrite raw takes.

Start with converter loopback and hardware bypass captures at low level. Measure round-trip delay, channel gain difference, polarity, frequency response, harmonic floor and noise floor. The capture chain must be measurably better than the behavior being identified; otherwise report only a bound.

The factory XT procedure uses XLR pin 3 positive and pin 2 negative. Document the actual polarity of the chosen cable/interface route rather than assuming a modern pin convention. [HW01 PDF p32]

Calibrate analog RMS voltage for a known sine playback level at the actual interface gain setting. Record both DAC and ADC calibration. Retain headroom below whichever clips first: the converter or hardware. Read the factory load tests for context; do not apply their +24-level tests before confirming the interface's limits.

### Keep level conventions explicit

`V_rms = 0.775 * 10^(dBu/20)`. Therefore +4 dBu is about 1.228 Vrms. `dBV = 20*log10(V_rms/1 V)`; zero dBV and zero dBu differ by about 2.218 dB.

If a full-scale sine with **sample peak = 1.0** would produce `F` dBu at the hardware input, a sine with sample-peak level `D` dBFS produces `F + D` dBu. Example only: if `F = +18 dBu`, -14 dBFS peak corresponds to +4 dBu. This is not an assumed calibration for your interface.

A digital RMS meter referenced to amplitude 1.0 reads a full-scale sine as -3.0103 dBFS RMS. Some tools compensate this for sine calibration; others do not. Record the tool's convention and the actual waveform peak/RMS. Analog meter zero, digital full scale and compressor threshold are three separate references.

## 2. Capture a useful measurement grid

The grid is a starting proposal; reduce it when redundant and expand it where residual errors show unexplained behavior. Allow enough settling and recovery between points to avoid measuring a previous stimulus's state.

| Test | Starting matrix | Save / examine |
| --- | --- | --- |
| Settled static transfer | 1 kHz stepped sine; both knees; ratios 1, 2, 4, 10, infinity, -5, -2, -1; several thresholds; 0.25-0.5 dB steps near the knee | Input/output RMS after settling, actual ratio slope, knee shape, overload limit |
| Knob mapping | Photograph and measure repeatable marked positions and intermediate positions; approach from both directions | Position versus effective threshold, inverse ratio and output gain; repeatability |
| Small-signal audio response | Compression inactive; bypass and active; makeup -20/0/+20 dB; several input levels | Magnitude, phase and delay relative to loopback; level/load dependence |
| Compressed audio response | Several settled gain reductions, such as 3/6/12/20 dB | Fundamental response and harmonics; not an LTI impulse response for every state |
| Attack/release | Amplitude-switched carrier, with both plateaus above noise; 100 Hz, 1 kHz, 10 kHz; 1/3/6/10/20/30 dB steps where headroom permits | Full envelope, overshoot, residual ripple, recovery; exact step times and endpoints |
| Program history | Short burst, sustained tone and repeated bursts followed by the same release segment | Differences caused by prior duration and duty cycle |
| Detector sensitivity | Main path held constant; frequency-varying external detector stimulus at calibrated RMS level | Gain-reduction threshold versus detector frequency; separate audio path from detector weighting |
| Harmonics / IMD | Several frequencies, levels and GR settings; polarity variants; include SMPTE-style two-tone stimulus | Harmonic order, intermodulation, residual noise and gain-dependent distortion |
| Noise / feedthrough | No input under documented termination; several makeup gains; changing external detector signal | Noise spectrum, mains components, gain-dependent noise, control leakage |
| Meter behavior | Known input/output steps and measured GR | Segment thresholds, mode behavior, attack/recovery, peak persistence |

Do not exceed calibrated analog/converter limits to fill a proposed grid. A long sweep through an actively compressing unit is not a conventional linear transfer function. Use settled stepped tests and state the detector's behavior during each point. Keep fundamental response separate from the total RMS that includes harmonics/noise.

For amplitude steps, preserve oscillator phase through the transition and retain the exact stimulus. A step intentionally has broadband content, so distinguish switching artifacts from the hardware response. For extracting gain from a carrier, demodulate/average with a documented analysis window rather than dividing sample-by-sample around sine zero crossings.

For timing reports, state whether time is measured to a 10/90% point, a 1/e point, a dB tolerance around the settled value, or another criterion. Report multiple criteria when useful. The brochure's timing examples alone do not establish the criterion.

## 3. Use factory acceptance tests as anchors

The saved service manual, HW01 PDF pp29-33, includes test-point scaling, symmetry adjustments and external behavior tests. Treat it as a historical procedure with stated conditions. Useful external checks include threshold/infinity behavior, OverEasy's output near the nominal threshold, level-meter calibration at +4, response under specific loads and separate distortion tests.

Do not change internal trims before obtaining an initial dataset. If a qualified service adjustment is made later, capture a separate before/after dataset and retain both. Copying a tolerance band into the model is not a substitute for measuring the actual specimen.

## 4. Fit and compare candidate models

Start with calibrated level mapping, static curves and a physically informed detector. Use the legacy VCA/RMS documentation and actual circuit revision. Keep a generic compressor implementation as a baseline for comparison.

Fit program-dependent timing to the measured envelope family. Then inspect residual differences in linear response, symmetry-related harmonics, saturation, noise and control feedthrough. Add behavior only where repeatable data supports it. Separate transformer behavior from gain-cell behavior if the specimen includes the option.

Reserve complete musical takes and some measurement settings for validation. Do not tune every parameter against the same captures later used to claim accuracy. The grey-box paper provides one possible identification approach; machine learning is optional.

## 5. Validate within the DAW

Create a dedicated reference session containing aligned dry, hardware return, loopback and candidate-plugin tracks. Preserve raw captures and perform level/delay compensation on comparison tracks. Leave perceptual comparisons blind where practical and use output matching to reduce loudness bias.

Use isolated kick/snare, bass, vocals, drum bus and full-mix excerpts owned or licensed for testing. Compare envelope and transients as well as steady tone curves. A waveform null is useful after alignment, but analog noise and time variation prevent a perfect null; report residual spectra and audible differences alongside a gain/envelope error measure.

When a stereo model is built, compare L-only, R-only, matched mono, independent channels and anti-phase stereo. Verify power-based linking and its calibration. One mono hardware unit cannot directly establish the behavior of a strapped pair; treat the stereo implementation as provisional until the linking evidence is sufficient.

Run host checks at 44.1/48/88.2/96 kHz, multiple buffer sizes, real-time playback and offline render. Verify state recall, automation, sidechain routing, bypass transitions, reset behavior, denormals and silence recovery. If oversampling is offered, compare aliasing, latency, phase and CPU cost. Report latency correctly and compensate any dry/wet blend.

All of these host checks are future development work. The user has selected JUCE/C++ and a faithful hardware faceplate; see [instructions.md](../instructions.md). Optional parallel mix, sidechain filtering, linked stereo and oversampling should be assessed after the mono hardware behavior is characterized, without changing the faithful front panel.

## Dataset organization

Create a future `Captures` folder outside this source library or in a clearly separate subfolder. Suggested layout: `Raw`, `Loopback`, `Processed_For_Analysis`, `Graphs`, `Sessions`, `Metadata`. Never label a plugin render as a hardware capture.

Name takes with unit ID, date, sample rate, connection path, knee, threshold, ratio, makeup, stimulus and repetition. Store a copy of [capture metadata](capture_metadata_template.json) with every group. Include software versions and the exact analysis script revision so graphs can be reproduced.
