# Electronics Engineer Roadmap

> From Ohm's Law to custom silicon. Estimated time: 6–18 months depending on pace.

## Stage 0: The Lab Setup (Week 1)

Before writing a single line or soldering a joint, get your environment ready.

**Physical lab:**
- [ ] Multimeter (UNI-T UT61E or Fluke 101)
- [ ] Breadboard + 200pc jumper wire kit
- [ ] Component kit: resistors, capacitors, LEDs, diodes, transistors
- [ ] Arduino Uno or ESP32 DevKit
- [ ] USB power supply

**Software:**
- [ ] Arduino IDE 2 or VS Code + PlatformIO
- [ ] Falstad simulator (browser, no install)
- [ ] LTspice (Windows/Mac, free)
- [ ] KiCad 8

---

## Stage 1: Circuits Fundamentals (Months 1–2)

**Theory:**
- [ ] Voltage, current, resistance — physical intuition (not just formulas)
- [ ] Ohm's Law, power calculations
- [ ] Kirchhoff's Voltage Law (KVL) and Current Law (KCL)
- [ ] Series vs parallel circuits
- [ ] Voltage dividers, current dividers
- [ ] Thevenin & Norton equivalents
- [ ] Superposition theorem

**Components:**
- [ ] Resistors: color codes, tolerances, SMD markings, power ratings
- [ ] Capacitors: types, ESR, voltage ratings, polarization
- [ ] Inductors: self-inductance, mutual inductance, saturation
- [ ] Diodes: IV curve, forward voltage, Zener, Schottky, TVS
- [ ] Transistors: BJT (NPN/PNP), MOSFET (N/P channel), biasing

**Practice:**
- [ ] Build 10 breadboard circuits using Falstad first, then real hardware
- [ ] Measure everything with your multimeter — never trust theory alone

**Milestone:** Build a variable power supply with voltage divider + transistor

---

## Stage 2: Analog Electronics (Months 2–4)

**AC Circuits:**
- [ ] Complex impedance (Z = R + jωL - j/ωC)
- [ ] RC filters (low-pass, high-pass, band-pass)
- [ ] LC filters and resonance
- [ ] Decibels and frequency response (Bode plots)
- [ ] AC coupling and blocking capacitors

**Amplifiers:**
- [ ] BJT common-emitter amplifier design
- [ ] MOSFET amplifier stages
- [ ] Small-signal models
- [ ] Gain, bandwidth, input/output impedance

**Op-Amps:**
- [ ] Ideal vs real op-amp parameters
- [ ] Inverting, non-inverting, unity-gain buffer
- [ ] Summing amplifier, differential amplifier
- [ ] Integrator, differentiator
- [ ] Active filters (Sallen-Key, MFB)
- [ ] Comparator applications
- [ ] Schmitt trigger (hysteresis)

**Simulation practice:**
- [ ] LTspice AC analysis (.ac sweep)
- [ ] Transient analysis (.tran)
- [ ] Noise analysis (.noise)

**Milestone:** Design and simulate a 4th-order active low-pass filter in LTspice

---

## Stage 3: Digital Electronics (Months 3–5)

**Logic:**
- [ ] Boolean algebra, De Morgan's theorems
- [ ] Logic gates: AND, OR, NOT, NAND, NOR, XOR
- [ ] Combinational logic: adders, MUX, DEMUX, decoders, encoders
- [ ] Sequential logic: latches, D flip-flop, JK flip-flop
- [ ] Counters, shift registers, FSMs
- [ ] Logic families: CMOS, TTL, LVDS — voltage levels, drive strength

**Microcontroller Basics:**
- [ ] Harvard vs von Neumann architecture
- [ ] CPU, memory map, peripherals
- [ ] GPIO, ADC, DAC, PWM, timers
- [ ] Interrupts and ISRs
- [ ] Communication: UART, I2C, SPI

**HDL & FPGAs (Optional at this stage):**
- [ ] VHDL or Verilog syntax basics
- [ ] Implement an FSM in HDL
- [ ] Try Xilinx Vivado with a cheap Basys3 board

**Milestone:** Build a digital combination lock with a 7-segment display

---

## Stage 4: PCB Design (Months 4–6)

**Schematic Capture:**
- [ ] Component symbols and libraries
- [ ] Hierarchical schematics
- [ ] Net names and labels
- [ ] Power symbols and PWR_FLAGS
- [ ] Annotation and ERC (Electrical Rules Check)

**PCB Layout:**
- [ ] Footprint creation
- [ ] Board outline and constraints
- [ ] Component placement strategy
- [ ] Routing: auto vs manual, interactive
- [ ] Ground planes and copper pours
- [ ] Via types: through-hole, blind, buried, micro
- [ ] Design rule checking (DRC)
- [ ] Generating Gerbers and BOM

**PCB Best Practices:**
- [ ] Decoupling capacitor placement (as close as possible to IC)
- [ ] High-current path tracing
- [ ] Thermal relief and thermal pads
- [ ] Silkscreen and assembly drawings
- [ ] Test points

**Manufacturing:**
- [ ] Order your first 5-piece prototype from JLCPCB
- [ ] Hand solder or use JLCPCB assembly

**Milestone:** Design, order, and assemble a custom ESP32 breakout board

---

## Stage 5: Power Electronics (Months 6–9)

**Linear Regulators:**
- [ ] LDO operation, dropout voltage, PSRR
- [ ] Heat dissipation: P = (Vin - Vout) × I
- [ ] Bypass capacitor selection

**Switching Regulators:**
- [ ] Buck converter: operation, inductor selection, duty cycle
- [ ] Boost converter: operation, output diode, ripple
- [ ] Continuous vs Discontinuous conduction mode (CCM vs DCM)
- [ ] Feedback and compensation: Type II, Type III networks
- [ ] EMC: layout, filtering, snubber circuits

**Battery Systems:**
- [ ] Li-Ion/LiPo chemistry: voltage curves, C-rating
- [ ] Battery management: CC/CV charging, protection circuits
- [ ] Fuel gauging: coulomb counting, impedance tracking
- [ ] Battery pack design: series/parallel cells

**Power ICs:**
- [ ] LM2596, TPS54360, LT8645S (buck)
- [ ] MT3608, XL6009, LT3580 (boost)
- [ ] BQ24040, MCP73831 (Li-Ion chargers)
- [ ] BQ27441, MAX17048 (fuel gauges)

**Milestone:** Design a USB-C PD powered dual-output (5V/3.3V) regulator PCB

---

## Stage 6: Embedded Systems Deep Dive (Months 6–12)

**Bare-metal Programming:**
- [ ] Memory-mapped I/O, registers, bit manipulation
- [ ] Startup code, vector tables, linker scripts
- [ ] Interrupts: NVIC, priority, preemption
- [ ] DMA: memory-to-memory, peripheral-to-memory
- [ ] Clock configuration: PLLs, prescalers, HSE/HSI

**RTOS:**
- [ ] FreeRTOS: tasks, scheduler, context switching
- [ ] Inter-task communication: queues, semaphores, mutexes
- [ ] Memory management: heap, stack, fragmentation
- [ ] Real-time constraints, WCET analysis

**Communication Deep Dive:**
- [ ] UART: baud rate, framing, flow control
- [ ] I2C: clock stretching, multi-master, 10-bit addressing
- [ ] SPI: modes (CPOL/CPHA), DMA transfers
- [ ] CAN: arbitration, error handling, bus timing
- [ ] USB: descriptors, CDC, HID, DFU

**Milestone:** Write bare-metal firmware for STM32 that reads IMU via SPI + sends data via CAN

---

## Stage 7: Signals & RF (Optional, Months 9–15)

**Signal Processing:**
- [ ] Nyquist-Shannon theorem, aliasing
- [ ] Discrete Fourier Transform (DFT) / FFT
- [ ] FIR and IIR filter design
- [ ] Convolution, correlation, spectral analysis
- [ ] Z-transform and system analysis

**RF Fundamentals:**
- [ ] Transmission lines: characteristic impedance, reflection
- [ ] Antenna theory: dipole, patch, gain, radiation pattern
- [ ] Link budget: path loss, sensitivity, margin
- [ ] RF matching: Smith chart, lumped components
- [ ] EMC: emissions, immunity, FCC/CE basics

**SDR Practice:**
- [ ] RTL-SDR: receive FM, aircraft ADS-B, weather satellites
- [ ] GNU Radio: build a signal processing flowgraph
- [ ] Decode various protocols: APRS, 433MHz sensors, LoRa

**Milestone:** Receive aircraft ADS-B data with RTL-SDR + decode with dump1090

---

## Resources by Stage

| Stage | Book | Course |
|-------|------|--------|
| 1–2 | Make: Electronics (Platt) | Khan Academy EE |
| 2–3 | The Art of Electronics (Horowitz) | MIT 6.002 OCW |
| 3 | Digital Design (Mano) | Nand2Tetris |
| 4 | KiCad Like a Pro | Phil's Lab (YouTube) |
| 5 | Switching Power Supply Design (Pressman) | TI Power Training |
| 6 | Definitive Guide to ARM Cortex-M | FastBit Embedded Academy |
| 7 | Software Defined Radio for Engineers | GNU Radio Academy |
