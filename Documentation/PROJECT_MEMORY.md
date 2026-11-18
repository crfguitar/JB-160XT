# Project memory - JB 160XT

Last updated: 2026-11-15.

## Project overview and user intent

The project is in the research and information-gathering stage for the highest-quality practical VST recreation of a dbx 160XT compressor. The user owns a physical analog unit and already had the manual as 28 JPEG scans in `DBX-160xt-manual`. That folder is preserved. The user has now explicitly chosen a JUCE/C++ build and clarified the visual target as a faithful pixel-for-pixel representation of the real 160XT, alongside the most faithful practical digital reproduction of its analog behavior. No compressor implementation, hardware characterization session or specific DAW configuration has been established in this workspace.

The user requested broad web documentation covering DSP, response/transfer curves, circuit information and high-quality plugin visual references, organized cleanly into subfolders. Their standing instruction is to maintain detailed project memory: overview, problems, resolutions and future work within the DAW.

The root [instructions.md](instructions.md) is a concise brief covering the hardware target, fidelity priorities, unusual research caveats and links to detailed references. The user challenged the original 2060-word version as overprescriptive; it was shortened to remove generic engineering rules and premature architectural guidance. They prefer high-signal project instructions that trust a capable assistant's judgment and avoid unnecessary context overhead. Preserve that preference in future revisions. A calibrated straight-on reference and hardware measurements are still needed before claiming pixel-perfect or hardware-matched completion.

## What is now available

The root README links to a source index, findings brief, curve guide, schematic guide, plugin gallery and hardware measurement plan. Downloaded originals are separated from derived views. The source manifest records URLs, retrieval timestamps, sizes, SHA-256 checksums, PDF page counts, image dimensions and failed access attempts. Native extractions and text snapshots have separate provenance. See `00_Index/VERIFICATION_REPORT.md` for the final audit and counts.

Core evidence includes the complete 41-page Model 160X/XT service compilation, with the 160XT Test Procedure Revision 02 dated 1990-08-15 at PDF pp29-33. The seven-file archive called XT schematics includes roughly 5000-pixel-wide source scans, overlapping regions and a drawing explicitly marked 160X. A page guide prevents treating those as seven unique XT sheets.

The analog reference set includes legacy 2150 VCA and 2252 RMS detector datasheets, original dbx VCA application guidance, relevant THAT design notes, successor VCA comparisons and three foundational Blackmer patents. DSP papers cover general compressor design, RMS timing, VCA nonidealities and grey-box identification. These supply methods and equations; they are not a disclosed exact XT plugin algorithm.

Overloud Comp160 was the explicit XT plugin model found. The official manual contains native 1900-pixel-wide UI images, including XT. The vendor says its reference XT had an optional transformer. Waves, UA and NI original-160-family references are kept as comparators, with model scope marked. The official hardware image is 4096 x 2271. Artwork remains reference material.

## Important technical findings to retain

- Preserve the distinction between original 160/VU, X, XT and A revisions.
- The XT has program-dependent timing, OverEasy/hard-knee selection, and ratios extending through infinity into negative compression.
- The documented attack/release examples are incomplete behavioral anchors, not fixed attack/release presets.
- Separate audio frequency response, detector sensitivity and input/output transfer curves. The XT has no front-panel audio EQ; external detector EQ changes broadband compression behavior.
- Legacy 2150 control scaling differs from successor 218x data. Confirm the actual hardware parts and circuit scaling before choosing a model.
- Factory tests provide a useful OverEasy output anchor near threshold, separate from a generic quadratic knee assumption.
- True power summing, meter reference, connector polarity, output loading and optional transformer configuration matter to capture interpretation.
- Published gain-reduction LED counts disagree; inspect the physical panel.
- No calibrated public XT measurement dataset with sufficient settings was verified, and none of the user's hardware has been measured yet.

## Problems encountered and how they were handled

The session was interrupted unexpectedly; the cause was not established. Work resumed from saved files without requiring the user to repeat the task.

Unavailable placeholder proxy settings initially prevented downloads. A per-script direct connection worked without changing the machine's global configuration. Several sites returned 403/503 or HTML with a successful status code. The downloader validates content, follows published direct-download links where available, and records inaccessible references instead of creating corrupt PDFs.

The first service-manual mirror omitted the final sections. Visual inspection and page-count comparison exposed the problem; a complete 41-page source replaced it. The original university DSP-paper URL also returned non-PDF content; the author's alternate document mirror supplied the paper. Native vendor-PDF images improved on smaller web screenshots.

The schematic archive is mixed and partly repetitive. Original scans remain intact, with model and overlap caveats. Text extraction is searchable but does not correct bad OCR. Four alternative/index URLs remained unavailable for direct local download; substantive equivalent references are retained where available. Details are in `00_Index/RESEARCH_LOG_AND_GAPS.md`.

## Planned future DAW and development work

1. Identify the exact physical revision, transformer option and any service modifications. Photograph front/rear panels and record repeatable knob positions.
2. Select and document the DAW/interface capture chain. Calibrate DAC/ADC levels, digital sine peak/RMS conventions, polarity, round-trip latency and converter response before recording hardware tests.
3. Create a dedicated reference session with dry, loopback, hardware-return and analysis tracks; preserve unprocessed captures and metadata.
4. Capture settled hard-knee/OverEasy curves across ratios and thresholds, including negative ratios. Measure control mapping rather than assuming panel interpolation.
5. Capture program-dependent attack/release families, overshoot, detector ripple, frequency weighting and history dependence.
6. Characterize audio-path magnitude/phase, headroom, harmonic/IMD behavior, noise and control feedthrough across useful levels, gains and output loads.
7. Fit an interpretable mono model, then add only measured nonidealities. Keep transformer behavior and optional digital enhancements separately identifiable.
8. Compare held-out musical captures in the DAW using calibrated, time-aligned and level-matched tracks. Use envelope, spectral and listening comparisons; do not promise an exact analog null.
9. Validate sample-rate and buffer-size independence, state recall, automation, sidechain routing, bypass, silence recovery and offline render. Assess oversampling/latency/CPU if implemented.
10. Validate stereo linking against the evidence and ideally a strapped pair. One owned mono unit cannot establish every pair-specific behavior. Decide on parallel mix, sidechain filtering and other extensions after the core model is credible.

These are proposed next steps, not completed work or a user-approved final feature list. No DAW project was modified, no plugins were installed, no paid sources were purchased and no internal hardware adjustments were made during this research pass.

## Build stage - first working plugin (2026-11-15)

The user confirmed JUCE is at `C:\JUCE` (version 9.0.1) and that their unit has no output transformer. A first plugin now exists: `CMakeLists.txt`, `Source/` (DSP in `Source/DSP`, faceplate in `Source/UI`), `Tests/dsp_tests.cpp`, `Tools/render_panel.cpp`, `Assets/fonts` (Michroma, OFL). Build with `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` then `cmake --build build --config Release`. Outputs land under `build/JB160XT_artefacts/Release`. Nothing has been installed into a system plugin folder.

### What the schematic settled

Read from the native scans (drawing 160XT-00-01-02-01); values are quoted in `Source/DSP/Circuit160XT.h`.

- **Hard knee** is the precision rectifier around U5 (R74/R77, CR11/CR12 in the loop). **OverEasy** is the U6 path: CR10 open-loop into R75, biased on by one diode drop from CR5-CR7. The diode's exponential turn-on is the OverEasy curve. The model gives -5.5 dB out for 0 dB in and +0.05 for +20 in at infinity:1, inside the factory limits (-6 +/-2, 0 +/-1.5).
- **Timing** is the 2252-type detector with THAT's non-linear capacitor around U8 (C41 10 uF, C47 220 nF, C45 47 nF, R60 8M2, CR8/CR9). Fast time constant about 35 ms, which reproduces the spec: 125 dB/s release and roughly 15/5/3 ms attack for 10/20/30 dB steps. Small slow changes see about 5.6 times more capacitance.
- **Ratio pot** R79 with R80 8k25 / R81 4k32 and the R89 load gives 2:1 at the 2:1 mark and infinity at the infinity mark (after the R82 trim), -1:1 at the end stop. All three pots turn 300 degrees.
- **Strapping**: both detectors share the timing node, so linked stereo sums power.
- `SCHEMATIC_GUIDE.md` calls archive page 6 a 160X drawing; its title block says DBX 160XT and it carries the ratio, gain, strap and meter circuits.

### Decisions made without the user, open to change

- 0 dBFS sine = +22 dBu (so -18 dBFS = +4 dBu = meter "0").
- SLAVE on a stereo instance links the pair; SLAVE on a mono instance gives unity gain, as the hardware does with nothing on the strap jack. Default is SLAVE out (dual mono).
- Parameters are pot rotations; the host text shows the dB or ratio the circuit model gives.
- Output gain reaches +/-21.5 dB at the end stops (circuit value; panel says 20, factory allows +/-3).
- No output clipping, VCA distortion, noise or external detector input yet.

### Problems met

- The ratio legend at 3:1, 4:1, 6:1 and 10:1 sits 10-15 degrees anticlockwise of where the circuit model puts those ratios (model reads 2.8, 3.5, 5.3, 8.6 at those marks). Needs a hardware measurement to say which is right.
- JUCE 9 images are Direct2D-backed by default; writing pixels while a drawing context is open on one is lost. The metal texture uses a software image filled before drawing.
- `front.jpg` is only 3588 px wide and shot slightly off-axis, so positions are good to a few pixels and knob centres were taken from the printed ticks, not the caps. The lettering uses Michroma thickened with a stroke, fitted to each measured legend box; it is close to, not identical to, the panel face.

### Next

1. User listens and looks in the DAW; adjust default size and anything that reads wrong.
2. Hardware captures (static curves at the ratio marks first, then step envelopes) to settle the ratio legend, pot end zones, OverEasy curve and interface calibration.
3. A sharper, squarer front photo to tighten the faceplate; consider redrawing the lettering as matched outlines.
4. Then: VCA nonidealities, output headroom, noise, external detector input, host validation across sample rates.

### Sidechain (2026-11-15)

The user asked for easy sidechaining (kick ducking 808s). Implemented as the hardware's rear-panel DETECTOR INPUT: a VST3 sidechain bus plus a Detector Input parameter (Internal / External), chosen from a right-click menu on the panel so the front panel stays faithful. External with no sidechain bus falls back to internal. Not yet confirmed working in FL Studio's wrapper routing.

