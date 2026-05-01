# Electronics Quick Reference

## Fundamental Formulas

```
Ohm's Law:              V = I × R
Power:                  P = V × I = I²R = V²/R
Energy:                 E = P × t  [Joules = Watts × seconds]

Series resistors:       R_total = R1 + R2 + R3 + ...
Parallel resistors:     1/R_total = 1/R1 + 1/R2 + ...
Voltage divider:        Vout = Vin × R2/(R1+R2)

Series capacitors:      1/C_total = 1/C1 + 1/C2 + ...
Parallel capacitors:    C_total = C1 + C2 + C3 + ...
Capacitor charge:       Q = C × V
Capacitor current:      I = C × dV/dt

Series inductors:       L_total = L1 + L2 + ...
Parallel inductors:     1/L_total = 1/L1 + 1/L2 + ...
Inductor voltage:       V = L × dI/dt

RC time constant:       τ = R × C  (63.2% charge/discharge)
RL time constant:       τ = L / R
LC resonance:           f₀ = 1/(2π√LC)
Bandwidth (RC):         f₋₃dB = 1/(2πRC)

Decibels (voltage):     dBV = 20 × log₁₀(V₂/V₁)
Decibels (power):       dBW = 10 × log₁₀(P₂/P₁)
Impedance (capacitor):  Xc = 1/(2πfC)
Impedance (inductor):   XL = 2πfL
```

## Component Color Codes

### Resistors (4-band)
```
Color    | Digit | Multiplier | Tolerance
---------|-------|------------|----------
Black    |   0   |     ×1     |
Brown    |   1   |    ×10     | ±1%
Red      |   2   |   ×100     | ±2%
Orange   |   3   |  ×1,000    |
Yellow   |   4   | ×10,000    |
Green    |   5   |×100,000    | ±0.5%
Blue     |   6   |  ×1M       | ±0.25%
Violet   |   7   |  ×10M      | ±0.1%
Grey     |   8   |            | ±0.05%
White    |   9   |            |
Gold     |       |    ×0.1    | ±5%
Silver   |       |    ×0.01   | ±10%

Memory trick: "BB ROY Great Britain Very Good Wife"
Black Brown Red Orange Yellow Green Blue Violet Grey White
```

### SMD Resistor Markings
```
3-digit:  "472" = 47 × 10² = 4700Ω
4-digit:  "1002" = 100 × 10² = 10kΩ
EIA-96:   "01A" = 100Ω (lookup table required)
```

## Standard Component Values

### E24 Resistor Series (5% tolerance)
```
1.0  1.1  1.2  1.3  1.5  1.6  1.8  2.0
2.2  2.4  2.7  3.0  3.3  3.6  3.9  4.3
4.7  5.1  5.6  6.2  6.8  7.5  8.2  9.1
(× any power of 10)
```

### Common Capacitor Values
```
Ceramic (decoupling): 100nF (0.1μF) — universal decoupling cap
Ceramic (bypass):     10nF, 1μF
Electrolytic (bulk):  10μF, 47μF, 100μF, 470μF, 1000μF
Timing:               Any value — calculate from RC formula
```

## Voltage References

```
Logic voltage levels:
  TTL:  LOW < 0.8V,  HIGH > 2.0V
  CMOS (3.3V): LOW < 0.9V, HIGH > 2.4V
  CMOS (5V):   LOW < 1.5V, HIGH > 3.5V
  LVDS: 350mV differential

Common supply rails:
  1.0V  — CPU core (sub-threshold, advanced nodes)
  1.2V  — DDR4/5 VDD
  1.8V  — IO buffers, many sensors
  2.5V  — Some DDR, legacy IO
  3.3V  — Most MCU IO, SPI flash, SD card
  5.0V  — Arduino, USB power, motors
  12V   — Motors, relays, automotive
  24V   — Industrial, servo drives
  48V   — PoE, 48V bus architecture

Li-Ion cell voltages:
  Full: 4.20V
  Nominal: 3.60–3.70V
  Empty: 3.00V (cutoff — below damages cell)
  Never above 4.25V (thermal runaway risk)
```

## Op-Amp Configurations

```
Inverting:          Vout = -(Rf/Rin) × Vin
Non-inverting:      Vout = (1 + Rf/Rin) × Vin
Voltage follower:   Vout = Vin  (Rf=0, Rin=∞)
Differential:       Vout = (R2/R1) × (V+ - V-)  [R1=R3, R2=R4]
Integrator:         Vout = -(1/RC) ∫Vin dt
Differentiator:     Vout = -RC × dVin/dt
Summing:            Vout = -(Rf/R1×V1 + Rf/R2×V2 + ...)
Instrumentation:    Vout = (1 + 2R/Rg) × (V+ - V-)
```

## Transistor Quick Reference

### NPN BJT (e.g., 2N3904, BC547)
```
Regions:
  Cutoff:     IB = 0 → IC = 0 (switch OFF)
  Active:     IC = β × IB (amplifier, β ≈ 100-300)
  Saturation: VCE_sat ≈ 0.2V (switch ON)

Switch design:
  IB_required = IC_max / β_min
  IB_actual = (Vin - 0.7V) / RB  →  RB = (Vin - 0.7) / IB_actual
  Use 10× overdrive: IB = 10 × (IC_max / β_min)

Key voltages:
  VBE ≈ 0.65–0.7V (forward biased)
  VCE_sat ≈ 0.1–0.2V
  Max VCE: check datasheet (typically 40V for 2N3904)
```

### N-Channel MOSFET (e.g., 2N7000, IRF540)
```
Regions:
  Off:    VGS < VTH  (VTH typically 1–4V)
  Linear: VDS < VGS - VTH (triode, switch on)
  Sat:    VDS > VGS - VTH (amplifier)

Switch design:
  Drive VGS to 10V+ for logic-level MOSFETs (fully on)
  Logic-level MOSFETs (IRLZ44, 2N7002): VGS = 5V sufficient
  Standard MOSFETs: need gate driver or 10-12V drive

RDS_on: on-resistance when fully on
  IRLZ44N: 22mΩ at VGS=5V → P = I² × 0.022
  Use heatsink if P > 1W
```

## PCB Design Rules (JLCPCB 2-layer)

```
Minimum trace width:        0.1mm (4mil) — avoid, use 0.15mm+
Recommended signal trace:   0.2mm (8mil)
Recommended power trace:    0.5mm–2mm (depends on current)
Trace width for current:    ~1mm per 1A (external, 10°C rise)
Minimum clearance:          0.1mm — increase for high voltage

Via drill diameter:         0.3mm minimum (0.4mm recommended)
Via annular ring:           0.15mm minimum
Via pad diameter:           drill + 2 × ring = 0.3 + 0.3 = 0.6mm

Board thickness:            1.6mm standard
Copper weight:              1oz (35μm) standard, 2oz available
Min hole size:              0.3mm
Min silkscreen text:        0.8mm height, 0.15mm line width

50Ω microstrip (1.6mm FR4, 1oz):  ~2.8mm trace width
100Ω differential:               ~0.2mm traces, 0.2mm gap (verify with calculator)
```

## Current Capacity

```
Wire gauge:
  AWG 30: 0.86A max (thin hookup wire)
  AWG 28: 1.0A
  AWG 26: 1.0A
  AWG 24: 3.5A  ← common robotics wiring
  AWG 22: 7A
  AWG 20: 11A
  AWG 18: 16A  ← motor wiring
  AWG 16: 22A
  AWG 14: 32A

PCB trace (1oz copper, 10°C rise, external):
  0.25mm: 0.5A
  0.5mm:  1.0A
  1.0mm:  1.7A
  2.0mm:  3.0A
  3.0mm:  4.5A
  Use Saturn PCB Calculator for exact values

Connector current ratings:
  JST PH 2mm:  2A per pin
  JST XH 2.5mm: 3A per pin
  XT30:        30A
  XT60:        60A
  Anderson SB50: 120A
```

## Communication Protocol Comparison

```
Protocol | Wires | Speed     | Distance | Devices | Notes
---------|-------|-----------|----------|---------|-------
UART     | 2     | ≤5Mbps    | <10m     | 2 only  | Async, no clock
I2C      | 2     | 100k-3.4M | <1m      | 128     | Addressed, multi
SPI      | 4     | 10-100M   | <1m      | N(+CS)  | Fast, no addr
CAN      | 2     | 1Mbps     | 40m      | 127     | Automotive robust
RS-485   | 2     | 10Mbps    | 1200m    | 32+     | Industrial
USB FS   | 4     | 12Mbps    | 5m       | 127     | Full speed
USB HS   | 4     | 480Mbps   | 5m       | 127     | High speed
UART 3V3 | 2     | 1Mbps     | <3m      | 2 only  | Most MCUs
```

## Quick Formulas for Common Tasks

### LED Resistor
```
R = (Vcc - Vf) / If
Common LEDs: Vf_red=2.0V, Vf_green=2.1V, Vf_blue=3.3V, Vf_white=3.2V
If = 10–20mA typical (5mA for indicator)
Example: 5V supply, red LED, 15mA
R = (5 - 2.0) / 0.015 = 200Ω → use 220Ω
```

### RC Filter Cutoff
```
f_c = 1 / (2π × R × C)
For 1kHz LPF: R=10kΩ → C = 1/(2π × 10k × 1k) = 15.9nF → use 15nF
For 100Hz LPF: R=10kΩ → C = 159nF → use 150nF
```

### Voltage Divider for ADC Input Scaling
```
Scaling 12V to 3.3V ADC:
Vout/Vin = R2/(R1+R2) = 3.3/12 = 0.275
Pick R2=10kΩ → R1 = R2 × (1/0.275 - 1) = 10k × 2.636 = 26.4kΩ → use 27kΩ
Verify: 12V × 10/(27+10) = 12 × 0.27 = 3.24V ✓ (slightly under 3.3V, safe)
```

### Battery Life Estimate
```
t = C_Ah / I_A  [hours]
Example: 2000mAh battery, 200mA average draw
t = 2.0Ah / 0.2A = 10 hours
Derate by 20% for real-world: ~8 hours

For ESP32 with deep sleep:
I_avg = (I_active × t_active + I_sleep × t_sleep) / (t_active + t_sleep)
I_avg = (160mA × 0.1s + 0.01mA × 9.9s) / 10s = 1.61mA
Battery life (1000mAh) = 1.0Ah / 0.00161A = 621 hours = 26 days
```

### PWM to Analog (RC Filter)
```
Vout = Vcc × duty_cycle
Ripple ≈ Vcc × T / (R × C)  (for RC >> T)
For 1% ripple on 5V output with f=1kHz:
0.05V ≈ 5V × 1ms / (R × C)
RC ≥ 100ms → R=10kΩ, C=10μF
```
