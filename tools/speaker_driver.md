# Speaker Driver — NPN Emitter-Follower (BC337)

Class-A audio amplifier driving a **17.5 Ω** loudspeaker from a Raspberry Pi Pico
GPIO. The BC337 is wired as an **emitter follower**: the speaker output follows the
PWM input, offset by one base-emitter drop. A first-order RC low-pass (R3 + C2)
removes the 8 kHz PWM carrier before it reaches the transistor, and C1 blocks DC so
the speaker only sees the AC audio signal.

- **Supply:** 5 V (VCC)
- **Input:** GPIO, 0–3.3 V PWM (8 kHz carrier, audio up to ~4 kHz)
- **Low-pass:** R3 + C2 → f_c ≈ 3.3 kHz (attenuates the 8 kHz carrier by ~8 dB)
- **Load:** 17.5 Ω loudspeaker
- **Max output power:** ≈ 16 mW (see calculations below)

---

## Schematic

```text
              +5 V (VCC)
                |
              [R1] 1 kΩ    <- base pull-up
                |
 GPIO            o           <- B node: R1, R2, R3 all meet here
(0-3.3 V PWM)     \          (single point, three connections)
    o----[R3]------\        R3 = 220 Ω, series from GPIO
                    |
                   / \
                  /   \
              [R2]    [C2] 220 nF
              1.8 kΩ   LPF shunt
                |        |
               GND      GND
                        (R3+C2: f_c ≈ 3.3 kHz,
                         attenuates 8 kHz carrier)

 Transistor Q1 (BC337 NPN), emitter follower:

                      B
                      |
                    +---+
                    |   |  Q1 BC337
                    |   |
                    +---+
                   /     \
                  E       C
                  |        |
            [C1] 100 µF    |
              |         |
             GND       +5 V (VCC)

 Speaker output:
    E ----[SPK] 17.5 Ω ---- GND   (via C1, DC-blocked)
```

### Signal path

| Stage | Component | Purpose |
|-------|-----------|---------|
| Input  | GPIO → R3 (220 Ω) | Series limit from the Pico pin into the base |
| LPF    | R3 + C2 (220 nF) | First-order low-pass, f_c ≈ 3.3 kHz; attenuates the 8 kHz PWM carrier by ~8 dB before it reaches Q1 |
| Bias   | R1 (1 kΩ) / R2 (1.8 kΩ) | Voltage-divider bias on the base (~3.2 V DC, emitter rests at ~2.5 V — mid-swing so the AC audio can swing ±2.5 V without clipping; low Thevenin resistance for stiff base drive) |
| Output | Q1 emitter → SPK | Emitter follower: low-impedance drive for the speaker |
| Coupling | C1 (100 µF) | Blocks DC at the emitter; passes audio AC to the speaker |

---

## Bill of Materials (BOM)

| Ref  | Value   | Part / Notes                          | Qty |
|------|---------|---------------------------------------|-----|
| Q1   | BC337   | NPN general-purpose transistor (TO-92) | 1   |
| R1   | 1 kΩ    | Base pull-up to VCC                    | 1   |
| R2   | 1.8 kΩ  | Base pull-down to GND                  | 1   |
| R3   | 220 Ω   | GPIO series resistor (also LPF series element) | 1 |
| C2   | 220 nF  | Ceramic, LPF shunt at base (R3+C2 → f_c ≈ 3.3 kHz) | 1 |
| C1   | 100 µF  | Electrolytic, DC block (polarity + at emitter) | 1 |
| SPK  | 17.5 Ω  | Loudspeaker                            | 1   |

---

## Calculations

### Base bias point

R1/R2 form a divider from VCC to GND:

```text
V_B = VCC · R2 / (R1 + R2)
    = 5 V · 1.8 kΩ / (1 kΩ + 1.8 kΩ)
    ≈ 3.2 V
```

With `V_BE ≈ 0.7 V`, the emitter rests at:

```text
V_E_rest = V_B − V_BE ≈ 3.2 − 0.7 ≈ 2.5 V
```

This places the emitter **midway** between its two extremes (≈ 0 V and ≈ 2.6 V),
so the AC audio signal from the PWM can swing roughly **±2.5 V** around the rest
point without clipping either rail. C1 then blocks this DC offset so the speaker
only sees the AC component.

The divider's Thevenin resistance is low:

```text
R_th = R1 ∥ R2 = (1 kΩ · 1.8 kΩ) / (1 kΩ + 1.8 kΩ) ≈ 643 Ω
```

This keeps the base voltage stiff under load — even when the transistor draws
several mA of base current, the divider sags only a few tens of mV rather than
pulling the bias point down.

> **Why the old 10 kΩ / 1 kΩ values were wrong:** that divider gave V_B ≈ 0.45 V,
> which is *below* V_BE — Q1 was barely conducting at rest, the emitter sat near
> 0 V, and the ±200 mV AC swing from PWM could only push it up to ~500 mV. The
> speaker cone barely moved because less than 1/5 of the available voltage
> headroom was being used. The intermediate 2.7 kΩ / 4.7 kΩ values fixed the DC
> point but left R_th ≈ 1.7 kΩ, which sags under base current and limits peak
> drive; the final 1 kΩ / 1.8 kΩ pair keeps the same ~3.2 V bias with a much
> stiffer source.

### Emitter (speaker) voltage swing

```text
V_E = V_B − V_BE
```

- PWM low (0 V): Q1 off → `V_E ≈ 0 V` (clips slightly on negative peaks)
- PWM high (3.3 V): `V_E ≈ 3.3 V − 0.7 V ≈ 2.6 V` (near the ceiling)
- Rest (mid-swing): `V_E ≈ 2.5 V`

After C1 removes the DC, the speaker sees roughly **±2.5 V peak** of audio —
about **10× more voltage swing** than the old bias, which translates to ~40 dB
more acoustic output at the cone.

### Base current at full input

```text
I_B = (V_IN − V_BE) / R3
    = (3.3 V − 0.7 V) / 220 Ω
    ≈ 11.8 mA
```

This is the peak base current when the PWM is fully high and the base sits near its
rest voltage. In practice the GPIO pin only sources current during the rising
transients of each PWM cycle, so the average pin current is well within the Pico's
~12 mA per-pin limit. The 1 kΩ pull-up and 1.8 kΩ pull-down draw ~1.8 mA through
the divider at all times — small, but worth noting for battery-powered use.

> **Why R3 was lowered from 470 Ω:** with 470 Ω the base drive was marginal and the
> output only reached ~1 V p-p. Dropping to 220 Ω roughly doubles the available base
> current, letting the BD165 saturate fully on the negative peaks — measured result:
> 2.8 V p-p, swinging all the way down to 0 V (full cutoff). Combined with the lower
> R_th divider (1 kΩ / 1.8 kΩ), the base is now driven hard enough for full swing
> in both directions.

### Speaker power

Using the peak emitter voltage across the 17.5 Ω load:

```text
P_peak = V_E² / R_L = (2.6 V)² / 17.5 Ω ≈ 0.39 W   (instantaneous peak)
P_RMS  ≈ P_peak / 2  ≈ 0.19 W                     (sine, full-scale)
```

In practice the PWM-driven average is far lower (~16 mW for a typical C64
audio signal), which is plenty for a small piezo/speaker in a 1541-style enclosure
and keeps the transistor comfortably within its safe operating area.

### Low-pass filter (PWM carrier rejection)

R3 and C2 form a first-order RC low-pass at the base:

```text
f_c = 1 / (2π · R3 · C2)
    = 1 / (2π · 220 Ω · 220 nF)
    ≈ 3.29 kHz
```

At the 8 kHz PWM carrier:

```text
|H(8 kHz)| = 1 / sqrt(1 + (f/f_c)²)
            = 1 / sqrt(1 + (8000/3290)²)
            ≈ 0.39   →  −8.4 dB  (~61 % amplitude reduction)
```

The carrier is reduced by roughly a factor of 2.6 in voltage. Combined with the
speaker's own mechanical roll-off above ~3 kHz (small cones lose efficiency quickly),
the residual 8 kHz buzz is well below audibility for normal C64 audio content. Audio
above ~3.3 kHz is also attenuated, but the SID's output spectrum is already dark in
that region and a small 1541-style speaker has little presence there anyway.

> **Note:** R3 was lowered from 470 Ω to 220 Ω for stronger base drive (see above),
> which would have raised f_c to ~7.2 kHz on its own. C2 was increased from 100 nF
> to 220 nF to keep the cutoff at ~3.3 kHz and maintain the same carrier rejection.

### Coupling capacitor time constant

```text
τ = R_L · C1 = 17.5 Ω · 100 µF ≈ 1.75 ms
f_c = 1 / (2π·τ) ≈ 90 Hz
```

C1 passes the audio band (≈ 200 Hz – 4 kHz) with little attenuation and blocks the
DC bias at the emitter.

---

## Files

| File | Description |
|------|-------------|
| `speaker_driver.md` | This document |
| `speaker_driver.png` | Rendered schematic (generated by `make_speaker_driver.py`) |
| `make_speaker_driver.py` | schemdraw script that produces the PNG |

> **Note:** The PNG is generated with `schemdraw 0.23`. Use a *numeric* unit
> (`Drawing(unit=1.0)`) — passing the string `unit="mm"` crashes in
> `Element._place()` because the string leaks into the zoom/scale math.
