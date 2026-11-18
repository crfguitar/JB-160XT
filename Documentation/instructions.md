# JB 160XT - coding assistant instructions

Build a **JUCE/C++ VST3** that recreates the **dbx 160XT**: a pixel-for-pixel front panel and the closest practical digital reproduction of its analog audio behavior. There is a lot of information here so we arent just wrapping a standard JUCE compressor and drawing a nice GUI and calling it a day. We are using the real hardware as the visual and sonic target, preserving it's layout, lettering, scales, knobs, buttons and indicators. Keep the faceplate faithful.

## Priorities

- Use the real hardware as the visual and sonic target. Preserve its layout, lettering, scales, knobs, switches and indicators. Keep the faceplate faithful; additional controls or a redesign are outside the current goal.
- Base audio behavior on the matching circuitry and calibrated hardware captures. Prioritize control mapping, OverEasy/hard-knee curves, program-dependent timing, negative ratios, and then supported response/nonlinearity details. Choose the implementation approach freely according to evidence and performance.

