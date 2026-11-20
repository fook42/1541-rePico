#!/usr/bin/env python3
"""Generate a stepper-motor click PCM file.

Per-click structure:
  - 3.5 kHz transient tick (decay ~0.4 ms)
  - 700 Hz body tone      (decay ~1.5 ms)
  - decaying broadband noise
  - total click duration: 8 ms
  - spacing between clicks: 120 ms  (≈41 clicks in 5 s)
  - per-click amplitude jitter: 0.8 – 1.0

Output: 8-bit unsigned mono PCM, sample rate configurable (default 16 kHz).
"""

import math
import random
import struct
import sys


def generate(
    sample_rate: int = 16_000,
    duration_s: float = 5.0,
    tick_hz: float = 2_400.0,
    tick_decay_ms: float = 7.0,
    body_hz: float = 550.0,
    body_decay_ms: float = 26.0,
    click_ms: float = 16.0,
    spacing_ms: float = 120.0,
    jitter_min: float = 0.85,
    jitter_max: float = 1.0,
    seed: int = 42,
) -> bytes:
    rng = random.Random(seed)

    

    click_samples = int(sample_rate * click_ms / 1000.0)
    spacing_samples = int(sample_rate * spacing_ms / 1000.0)

    # Per-sample exponential decay rates (time constant in samples)
    tick_decay = 1.0 / (sample_rate * tick_decay_ms / 1000.0)
    body_decay = 1.0 / (sample_rate * body_decay_ms / 1000.0)

    if (duration_s == 0):  # Special case: single click
        total_samples = int(click_samples)
        n_clicks = 1
        spacing_samples = 0
    else:
        total_samples = int(sample_rate * duration_s)
        n_clicks = total_samples // spacing_samples

    out = bytearray([128] * total_samples)
    for c in range(n_clicks):
        start = c * spacing_samples
        amp = rng.uniform(jitter_min, jitter_max)

        for i in range(click_samples):
            idx = start + i
            if idx >= total_samples:
                break

            t_s = i / sample_rate

            # 6 kHz transient tick (dominant at start)
            tick_env = math.exp(-i * tick_decay)
            tick = amp * 1.0 * tick_env * math.sin(2.0 * math.pi * tick_hz * t_s)

            # 1.2 kHz body tone (sustains the click tail)
            body_env = math.exp(-i * body_decay)
            body = amp * 0.85 * body_env * math.sin(2.0 * math.pi * body_hz * t_s)

            # decaying noise (gives it a mechanical "chirp" texture)
            noise_env = math.exp(-i * tick_decay * 1.2)
            noise = amp * 0.40 * noise_env * rng.uniform(-1.0, 1.0)

            sample = 128 + int(127 * (tick + body + noise))
            out[idx] = max(0, min(255, sample))

    return bytes(out)


def main():
    sample_rate = int(sys.argv[1]) if len(sys.argv) > 1 else 16_000
    duration_s = float(sys.argv[2]) if len(sys.argv) > 2 else 5.0    
    out_path = sys.argv[3] if len(sys.argv) > 3 else "step_8bit.pcm"

    data = generate(sample_rate=sample_rate, duration_s=duration_s)
    with open(out_path, "wb") as f:
        f.write(data)

    if (duration_s == 0 ):
        n_clicks=1
    else:
        n_clicks = int(duration_s * 1000.0 / 120.0)
    print(f"Wrote {out_path}: {len(data)} bytes ({sample_rate} Hz, {duration_s:.1f} s, ~{n_clicks} clicks)")


if __name__ == "__main__":
    main()
