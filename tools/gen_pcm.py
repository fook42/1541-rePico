#!/usr/bin/env python3
"""Generate 8-bit unsigned PCM sine-wave files.

Usage:
    python3 gen_pcm.py <frequency_hz> <output_file> [sample_rate] [duration_s]

Examples:
    python3 gen_pcm.py 1000 firmware/sounds/1khz_8bit.pcm
    python3 gen_pcm.py 2000 firmware/sounds/2khz_8bit.pcm 16000 5
    python3 gen_pcm.py 440 /tmp/a4.pcm 44100 10
"""

import math
import os
import struct
import sys


def generate_sine(freq_hz: float, out_path: str, sample_rate: int = 16000, duration_s: float = 5.0) -> None:
    num_samples = int(sample_rate * duration_s)
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)

    with open(out_path, "wb") as f:
        for i in range(num_samples):
            t = i / sample_rate
            sample = 128 + int(127 * math.sin(2 * math.pi * freq_hz * t))
            f.write(struct.pack("B", max(0, min(255, sample))))

    size = os.path.getsize(out_path)
    print(f"Wrote {out_path}")
    print(f"  Frequency:   {freq_hz:.1f} Hz")
    print(f"  Sample rate: {sample_rate} Hz")
    print(f"  Duration:    {duration_s} s ({num_samples} samples)")
    print(f"  File size:   {size} bytes")


def main() -> None:
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    freq_hz = float(sys.argv[1])
    out_path = sys.argv[2]
    sample_rate = int(sys.argv[3]) if len(sys.argv) > 3 else 16000
    duration_s = float(sys.argv[4]) if len(sys.argv) > 4 else 5.0

    generate_sine(freq_hz, out_path, sample_rate, duration_s)


if __name__ == "__main__":
    main()
