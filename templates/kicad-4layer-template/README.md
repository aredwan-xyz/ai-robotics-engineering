# KiCad 4-Layer Robotics PCB Template

When designing custom PCBs for robotics (motor drivers, dense MCU boards, RF IoT nodes), a standard 2-layer board is often insufficient. It leads to terrible ground return paths, electromagnetic interference (EMI), and difficult routing.

This template defines the standard configuration for a high-performance **4-layer PCB** in KiCad 7/8.

## The Stackup (JLC7628)

We use the standard JLC PCB 4-layer stackup (JLC7628) because it is incredibly cheap and highly reliable.

1. **Top Layer (Signal / Power Polygons):** All high-speed traces (SPI, USB, I2C) and high-current power polygons.
2. **Inner Layer 1 (Solid Ground Plane):** Unbroken 0V reference. Do not route any signals here.
3. **Inner Layer 2 (Power Plane / Split Planes):** 3.3V and 5V power planes. Can route slow signals here if absolutely necessary.
4. **Bottom Layer (Signal / Ground):** Secondary routing layer. Try to keep traces orthogonal to Top Layer traces to reduce cross-talk.

## Design Rules (DRC) Setup

Before routing, ensure your KiCad Board Setup matches these minimum manufacturing constraints (based on JLCPCB's 4-layer capabilities):

*   **Min Trace Width:** 0.127 mm (5 mil)
*   **Min Clearance:** 0.127 mm (5 mil)
*   **Min Via Size:** 0.45 mm outer diameter / 0.25 mm drill hole.
*   **Via-in-Pad:** DO NOT USE unless you are paying for epoxy filling and capping (expensive).

## High-Speed Routing Checklist

- [ ] **Impedance Control:** USB (90Ω differential) and CAN Bus (120Ω differential) traces must be calculated using a trace width calculator for the specific stackup.
- [ ] **Decoupling Capacitors:** Place 0.1μF capacitors as physically close to the MCU power pins as possible. Use vias to drop immediately to the GND plane.
- [ ] **Crystal Oscillators:** Place a ground guard ring around the crystal. Do not route any signals underneath the crystal on *any* layer.
- [ ] **Motor Drivers:** Keep high-current paths incredibly short and wide. Do not let motor return currents flow across the same ground plane area as your sensitive ADC references.
