# Generates the hardware measurement stimulus for the JB 160XT project.
# Standard library only. Usage: python make_stimulus.py <out.wav>
#
# Layout (mono, 44.1 kHz, 24-bit, 1 kHz sine throughout, phase-continuous):
#   silence 2 s
#   reference tone  -18 dBFS, 5 s
#   silence 2 s
#   static sweep    -50 .. -2 dBFS in 1 dB steps, 1 s each
#   silence 3 s
#   step envelopes  3 x [ -40 dBFS 2 s | high 2 s | -40 dBFS 3 s ], high = -24, -14, -4 dBFS
#   silence 2 s
import math
import sys
import wave

RATE = 44100
FREQ = 1000.0

segments = []  # (seconds, level in dBFS or None for silence, label)
segments.append((2.0, None, "silence"))
segments.append((5.0, -18.0, "reference tone"))
segments.append((2.0, None, "silence"))
for db in range(-50, -1):
    segments.append((1.0, float(db), "sweep"))
segments.append((3.0, None, "silence"))
for high in (-24.0, -14.0, -4.0):
    segments.append((2.0, -40.0, "step base"))
    segments.append((2.0, high, "step high"))
    segments.append((3.0, -40.0, "step release"))
segments.append((2.0, None, "silence"))

out = bytearray()
phase = 0.0
dphi = 2.0 * math.pi * FREQ / RATE
full = (1 << 23) - 1
t = 0.0
for seconds, db, label in segments:
    n = int(round(seconds * RATE))
    amp = 0.0 if db is None else 10.0 ** (db / 20.0)
    print("%8.3f s  %-14s %s" % (t, label, "" if db is None else "%+.0f dBFS" % db))
    for _ in range(n):
        v = int(round(amp * math.sin(phase) * full))
        out += v.to_bytes(3, "little", signed=True)
        phase += dphi
        if phase > 2.0 * math.pi:
            phase -= 2.0 * math.pi
    t += seconds

with wave.open(sys.argv[1], "wb") as w:
    w.setnchannels(1)
    w.setsampwidth(3)
    w.setframerate(RATE)
    w.writeframes(bytes(out))
print("total %.1f s -> %s" % (t, sys.argv[1]))
