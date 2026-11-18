# JB 160XT - handoff

Written 2026-11-16 for a fresh session picking this project up. Read this first, then [Documentation/instructions.md](Documentation/instructions.md) (the brief) and, for the full decision log, [Documentation/PROJECT_MEMORY.md](Documentation/PROJECT_MEMORY.md).

## What this is

A JUCE/C++ VST3 that recreates the user's own dbx 160XT compressor: a faithful front panel (no added controls, no redesign) and audio behaviour modelled from the real circuit. The user owns the hardware, uses FL Studio, and has no output transformer fitted.

## Where it stands

A working plugin exists and is installed. The user has tried it in FL Studio and said it is "working quite well".

| Item | State |
| --- | --- |
| DSP (detector, knees, ratio law, strapping) | Built from the schematic; all automated checks pass |
| Faceplate | Drawn in code, positions measured from the user's front photo |
| Sidechain (external detector) | Built, passing its DSP check, and confirmed working in FL Studio by the user (2026-11-16: "works spectacularly") |
| Installed copy | `C:\Program Files\Common Files\VST3\JB 160XT.vst3`, from the 2026-11-15 17:52 build |
| Hardware measurements | None taken |
| Version control | No git repository |

The user's last request was the sidechain feature, and they have since confirmed it works in FL Studio.

## Build and test

JUCE 9.0.1 is at `C:\JUCE`. Toolchain is CMake with Visual Studio 2022.

```
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\Release\jb160_dsp_tests.exe
```

| Target | Purpose |
| --- | --- |
| `JB160XT_VST3`, `JB160XT_Standalone` | The plugin; output under `build/JB160XT_artefacts/Release` |
| `jb160_dsp_tests` | Plain C++ checks against the factory test procedure and the published spec; no JUCE |
| `jb160_render_panel` | Renders the editor to a PNG and runs a short audio smoke test |

Render tool usage: `jb160_render_panel <out.png> [width] [name=value ...]`. Names are the parameter IDs (rotations 0 to 1, switches 0/1) plus `level`, `reduction` and `over` to light the meters. Stack the result against `Documentation/pictures-of-my-160xt/front.jpg` to check the faceplate.

Installing is a manual copy of `build/JB160XT_artefacts/Release/VST3/JB 160XT.vst3` into `C:\Program Files\Common Files\VST3`. The user has approved copying builds there. Copy-after-build is switched off in CMake. The copy fails if FL Studio has the plugin loaded.

## Code map

| File | Contents |
| --- | --- |
| [Source/DSP/Circuit160XT.h](Source/DSP/Circuit160XT.h) | Circuit model with component designators: pot laws, knee stage, timing node, meter detector, calibration |
| [Source/DSP/Compressor160XT.h](Source/DSP/Compressor160XT.h) | The compressor: one or two units, strapping, external detector, meter values |
| [Source/PluginProcessor.cpp](Source/PluginProcessor.cpp) | Parameters, buses (main plus sidechain), state |
| [Source/PluginEditor.cpp](Source/PluginEditor.cpp) | Knobs, buttons, LED logic, right-click menu |
| [Source/UI/Faceplate.cpp](Source/UI/Faceplate.cpp) | All panel artwork; geometry constants are in `Faceplate.h` |
| [Tests/dsp_tests.cpp](Tests/dsp_tests.cpp) | The checks |

The DSP headers have no JUCE dependency, which is why the tests build in seconds.

## How the model works

All of this was read from the schematic scans in `Documentation/01_Hardware_Service/XT_Schematics/Native_Scans` (drawing 160XT-00-01-02-01) and checked against the factory test procedure (service manual PDF pages 29-33).

- **Topology:** feed-forward. The input feeds both the VCA and the RMS detector.
- **Hard knee:** a precision rectifier around U5, so it is an exact `max(0, level - threshold)`.
- **OverEasy:** the U6 path, where diode CR10 turns on outside the feedback loop with one diode drop of bias. The diode equation is the curve. The model gives -5.5 dB out for 0 dB in at infinity:1; the factory limit is -6 +/-2.
- **Timing:** a 2252-type log-RMS detector with THAT's "non-linear capacitor" around U8. The fast time constant is about 35 ms, placed from the spec's 125 dB/s release. Small slow changes see about 5.6 times more capacitance. It is solved per sample with backward Euler and Newton iteration.
- **Ratio pot:** R79 with R80 8k25, R81 4k32 and the R89 load. The parameter the DSP uses is the compression slope: 0 is 1:1, 1 is infinity:1, 2 is -1:1.
- **Strapping:** both detectors share one timing node, so a linked pair sums power.
- **Pots** turn through 300 degrees. Threshold ticks are every 50 degrees, gain every 75, and the infinity mark is at +75 (rotation 0.75).

## Decisions I made that the user has not explicitly confirmed

- **Level calibration:** a full-scale sine is +22 dBu, so -18 dBFS lights the meter's "0".
- **Parameters are knob rotations** (0 to 1); the host text shows the dB or ratio the model gives.
- **SLAVE button:** on a stereo instance it links the channels. On a mono instance it gives unity gain, as the hardware does with nothing on the strap jack. Default is out, so stereo starts as two independent channels.
- **Sidechain switch:** a `detector` parameter (Internal / External) chosen from a right-click menu on the panel background, because the jack is on the rear panel and the front must stay faithful. External with no sidechain bus falls back to internal.
- **Output gain** reaches +/-21.5 dB at the end stops (the circuit value); the panel says 20.
- **Default window** is 1500 px wide, resizable at a fixed aspect ratio.

## Known issues and gaps

- **Ratio legend disagrees with the model.** At the printed 3:1, 4:1, 6:1 and 10:1 marks the model gives about 2.8, 3.5, 5.3 and 8.6. It agrees at 2:1, infinity and the negative ratios. Either the legend is approximate or part of that network is misread (the R82 trim and R83/R84 connections run off the scan). A static measurement at those marks would settle it.
- **Not modelled:** VCA distortion, noise, output clipping at +24 dBu.
- **Threshold LEDs in OverEasy:** amber starts 8.1 dB below threshold and red starts at threshold, from the comparator values. The manual's figure suggests amber may extend above threshold; unresolved.
- **Lettering** is Michroma (open licence, in `Assets/fonts`) thickened with a stroke and fitted to each measured legend box. Close to the panel face, not identical.
- **Faceplate precision** is limited by `front.jpg`: 3588 px wide and shot slightly off-axis, so positions are good to a few pixels. Knob centres were taken from the printed ticks.
- **`SCHEMATIC_GUIDE.md` is wrong about archive page 6.** It calls it a 160X drawing; the title block says DBX 160XT and it carries the ratio, gain, strap and meter circuits. The guide has not been corrected.

## Traps

- **JUCE 9 images are Direct2D-backed by default.** Pixel writes made while a drawing context is open on one are lost. The metal texture uses `juce::SoftwareImageType()` and is filled before any `Graphics` is created.
- **Schematic scans are about 5000 px wide** and get downscaled when viewed whole, which makes 1 and 10 look alike. Crop at native resolution with PowerShell `System.Drawing`; Python on this machine has no PIL.
- **PowerShell variables are case-insensitive** (`$S` and `$s` are the same variable).
- **Bash here-documents containing triple quotes fail** in this environment; write a script file and run it.

## Suggested next steps

1. (Done: sidechain confirmed working in FL Studio.)
2. Get the user's impressions of sound and look against the hardware.
3. Hardware captures, following `Documentation/07_Hardware_Measurement_Plan/MEASUREMENT_PLAN.md`: static curves at the ratio marks first, then step envelopes. These settle the ratio legend, the OverEasy curve, pot end zones and the dBFS-to-dBu calibration.
4. A sharper, squarer front photo to tighten the faceplate.
5. After measurements: VCA nonidealities, output headroom, noise, and host validation across sample rates.

## Working with this user

- Keep instruction documents short and specific; they cut an earlier brief down for being overprescriptive.
- Their global instructions ask for a detailed project memory: overview, problems, how they were solved, future plans. Keep `Documentation/PROJECT_MEMORY.md` current as work continues.
- The front panel stays faithful. New features go somewhere that does not alter it.
