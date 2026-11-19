#!/usr/bin/env python3
"""Interaktive Visualisierung der 8-Bit-PCM-Daten aus firmware/include/wavData.h.

Die Datei enthaelt ein Array:
    const unsigned char wav_data[] = { 0x00, 0x8b, ... };
    unsigned int wav_data_len = 441000;

Die Daten sind 8-Bit-unsigned PCM mit 8 kHz Samplerate (5 Sekunden Audio).

Verwendung:
    venv/bin/python tools/visualize_wavdata.py [--header PFADE]
                                               [--start-sec X] [--end-sec Y]
                                               [--output diagram.png] [--no-save]

Interaktion (matplotlibs Standard-Toolbar):
    - Zoom-Tool: im oberen Panel ein Rechteck ziehen -> beide Zeitpaneele zoomen
    - "home" (Haus-Symbol oder H): Zoom zuruecksetzen
    - Pfeiltasten: scrollen, +/-: zoomen
"""

import argparse
import re
import sys
from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt

SAMPLE_RATE = 8000  # 8 kHz
HEX_RE = re.compile(r"0x([0-9a-fA-F]{2})\b")
DEFAULT_HEADER = Path(__file__).resolve().parent.parent / "firmware" / "include" / "wavData.h"


def load_samples(header: Path) -> np.ndarray:
    """Liest das unsigned-char Array aus der Header-Datei ein."""
    text = header.read_text(encoding="utf-8", errors="replace")

    # Nur den Block zwischen "wav_data[] = {" und dem abschliessenden "};
    start = text.index("wav_data")
    block = text[start:]
    end = block.index("};")
    block = block[:end]

    values = np.fromiter((int(h, 16) for h in HEX_RE.findall(block)), dtype=np.uint8)
    return values


def plot(samples: np.ndarray, start_sec: float, end_sec: float, title: str) -> plt.Figure:
    """Zeichnet Gesamtverlauf (zoombar) und Amplitudenhistogramm.

    Zoomen: im oberen Panel mit dem Zoom-Tool der Toolbar ein Rechteck ziehen.
    """
    n = len(samples)
    duration = n / SAMPLE_RATE
    t = np.arange(n) / SAMPLE_RATE

    fig, (ax_main, ax_hist) = plt.subplots(
        2, 1, figsize=(14, 8), gridspec_kw={"height_ratios": [3, 1]}
    )
    fig.suptitle(f"{title} - {SAMPLE_RATE / 1000:.0f} kHz, 8-Bit PCM "
                 f"({duration:.2f} s, {n} Samples)")

    # 1) Gesamtverlauf - zoombar ueber die Toolbar
    ax_main.plot(t, samples, lw=0.3, color="tab:blue")
    # Start-Ausschnitt markieren
    ax_main.axvspan(start_sec, end_sec, color="tab:orange", alpha=0.25)
    ax_main.set_xlim(0, duration)
    ax_main.set_ylim(0, 255)
    ax_main.set_ylabel("Amplitude (0..255)")
    ax_main.set_title("Gesamtverlauf (im Panel ein Rechteck ziehen zum Zoomen, "
                      "Home-Taste zum Zuruecksetzen)")
    ax_main.grid(True, alpha=0.3)

    # 2) Amplitudenhistogramm (unabhaengig vom Zoom, immer Gesamtverteilung)
    counts = np.bincount(samples, minlength=256)
    ax_hist.bar(np.arange(256), counts, width=1, color="tab:purple")
    ax_hist.set_xlabel("Amplitudenwert (0..255)")
    ax_hist.set_ylabel("Anzahl Samples")
    ax_hist.set_title("Verteilung der Amplitudenwerte (gesamt)")
    ax_hist.grid(True, alpha=0.3, axis="y")

    fig.tight_layout(rect=(0, 0, 1, 0.97))
    return fig


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER,
                        help="Pfad zur wavData.h (Standard: %(default)s)")
    parser.add_argument("--start-sec", type=float, default=0.0,
                        help="Markierter Ausschnitt: Start in Sekunden (Standard: 0.0)")
    parser.add_argument("--end-sec", type=float, default=None,
                        help="Markierter Ausschnitt: Ende in Sekunden "
                             "(Standard: min(1.0, Gesamtdauer))")
    parser.add_argument("--output", type=Path, default=None,
                        help="PNG-Ausgabepfad (Standard: tools/wavData_view.png)")
    parser.add_argument("--no-save", action="store_true",
                        help="Nur anzeigen, nichts speichern")
    args = parser.parse_args()

    if not args.header.is_file():
        print(f"Header nicht gefunden: {args.header}", file=sys.stderr)
        return 1

    samples = load_samples(args.header)
    print(f"Geladen: {len(samples)} Samples aus {args.header}")

    duration = len(samples) / SAMPLE_RATE
    end = args.end_sec if args.end_sec is not None else min(1.0, duration)
    if not (0 <= args.start_sec < end <= duration):
        print(f"Ungueltiger Ausschnitt {args.start_sec}s..{end}s "
              f"(Dauer: {duration:.2f}s)", file=sys.stderr)
        return 1

    fig = plot(samples, args.start_sec, end, args.header.stem)

    if not args.no_save:
        out = args.output or (Path(__file__).resolve().parent / "wavData_view.png")
        fig.savefig(out, dpi=120)
        print(f"Gespeichert: {out}")

    # Interaktives Fenster
    plt.show(block=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
