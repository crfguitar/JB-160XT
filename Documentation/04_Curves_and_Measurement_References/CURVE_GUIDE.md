# Curves and measurement references

These are source graph pages and measurement-method references. There is no measured response of the user's hardware in this collection yet. A 300 dpi PDF rendering is a convenient view; it cannot recover detail absent from a scanned source.

| View | Source / scope |
| --- | --- |
| [Hard knee and OverEasy](Source_Graph_Pages/XT_Hard_Knee_and_OverEasy_Transfer_Curves.png) | HW01 PDF p9; manufacturer transfer illustrations, not an independently measured unit |
| [Ratios and negative compression](Source_Graph_Pages/XT_Ratio_and_Negative_Compression_Curves.png) | HW01 p10; ratio/control illustrations |
| [Frequency-weighted detector routing](Source_Graph_Pages/XT_External_Frequency_Weighted_Detector_Routing.png) | HW01 p14; external detector EQ routing, not an audio EQ curve |
| [2150 frequency response and noise versus gain](Source_Graph_Pages/2150_VCA_Frequency_Response_and_Noise_vs_Gain.png) | IC01 p3; component test circuit, not complete XT |
| [2150 control law and symmetry distortion](Source_Graph_Pages/2150_VCA_Control_Law_and_Symmetry_Distortion.png) | IC01 p5; legacy VCA behavior |
| [2150 distortion versus level/frequency](Source_Graph_Pages/2150_VCA_Distortion_vs_Level_and_Frequency.png) | IC01 p7; test levels, gains and circuits matter |
| [2252 tone burst and detector frequency response](Source_Graph_Pages/2252_Detector_Tone_Burst_Tracking_and_Frequency_Response.png) | IC02 p5; detector output voltage, not audio output response |
| [2181 successor comparison](Source_Graph_Pages/2181_Successor_VCA_Distortion_and_Spectrum_COMPARISON_ONLY.png) | IC04 p6; newer VCA, not evidence of parts installed in XT |

The user's existing [09.jpg](../DBX-160xt-manual/09.jpg) and [10.jpg](../DBX-160xt-manual/10.jpg) provide another scan of the operating curves. Their printed numbering differs from the service compilation. Original files were not altered.

[Floru AES4054](../03_DSP_Papers/Floru_1995_AES4054_RMS_Compressors_Limiters_Timing.pdf) contains theoretical and experimental RMS timing behavior. [AES4703](../03_DSP_Papers/Floru_1998_AES4703_RMS_Feedback_Compressor_Timing.pdf) extends the comparison to feedback arrangements. Their original download filenames use I/II in an order that does not match publication chronology; the local names use the actual year and paper number.

[DDMF Plugindoctor manual](DDMF_Plugindoctor_Manual.pdf) describes transfer/dynamics/linear analysis and external-hardware measurement routing. [REW signal generator](REW_Signal_Generator.txt) and [RTA/stepped sine](REW_Stepped_Sine.txt) explain complementary stimuli and measurement controls. Saved HTML originals sit alongside the searchable text snapshots; their page graphics may still require internet access.

## What remains to measure

- Frequency and phase versus level, gain, load, connector and amount of compression.
- Detector sensitivity versus frequency with the audio path held constant.
- Settled transfer curves for both knee modes and positive/infinite/negative ratios.
- Attack/release envelopes, overshoot and ripple for different level steps and prior histories.
- THD harmonics, IMD, noise and control feedthrough over a useful operating grid.

Do not turn brochure tolerances into an invented measured curve. Keep measured points, schematic predictions, theoretical curves and manufacturer illustrations labeled separately. Follow the [capture plan](../07_Hardware_Measurement_Plan/MEASUREMENT_PLAN.md) for the first dataset.
