# Hardware Debugging Guide

> "80% of hardware engineering is proving that it's a hardware problem, not a software problem (and vice versa)."

Debugging physical intelligence is hard because bugs can exist in the physical world (friction), the electrical world (noise), or the digital world (code).

## 1. The Multimeter (The Foundation)

Always start here before touching code.

- **Continuity Mode:** Verify traces aren't broken and that there are no shorts between VCC and GND. Do this *before* you ever apply power to a new PCB.
- **Voltage Checking:** Is the 3.3V rail actually 3.3V, or is it sagging to 2.8V under load?
- **Current Measurement:** If a chip gets hot, put your multimeter in series (Amps mode) to see how much current is being drawn.

## 2. The Oscilloscope (The Truth)

An oscilloscope shows you how voltage changes over time. It is the absolute truth of what is happening on a wire.

### What to look for:
- **Noisy Power Supplies:** Switch-mode power supplies (SMPS) can inject high-frequency noise. If your ADC readings are fluctuating wildly, check your 3.3V rail on the scope. You might need to add bulk capacitance or a ferrite bead.
- **Rise/Fall Times:** If your I2C communication fails at high speeds, look at the edges of the SDA/SCL signals. If they look like slow, rounded curves instead of sharp square waves, your pull-up resistors are too weak (resistance is too high).
- **Ground Bounce / Ground Loops:** If multiple high-current motors share a thin ground trace with your microcontroller, the return current can actually raise the "ground" voltage at the MCU, causing unexpected resets.

## 3. The Logic Analyzer (The Digital Truth)

While scopes are great for analog signals, logic analyzers are essential for debugging protocols (I2C, SPI, UART, CAN).

- **Decoding I2C:** If your sensor isn't responding, hook up a logic analyzer (even a cheap $10 Saleae clone). Does the MCU send the correct address? Does the sensor respond with an `ACK` (pulling SDA low)?
- **Decoding SPI:** SPI is faster and lacks built-in acknowledgments. Verify that the Chip Select (CS) line goes low *before* the clock starts, and verify you are using the correct SPI Mode (CPOL/CPHA).

## 4. Common Robotics Gotchas

### "My robot resets randomly when the motors turn on."
- **Cause:** Brownout. Motors draw a massive spike of current (stall current) when they start moving. This pulls the battery voltage down momentarily. If it drops below the MCU's minimum operating voltage, it resets.
- **Fix:** Add a large electrolytic capacitor (e.g., 1000μF) across the motor driver power inputs. Ensure the MCU is powered by an LDO/Buck converter with adequate input capacitance.

### "My servos jitter violently."
- **Cause:** Shared power supplies. Servos inject massive electrical noise back into the power lines.
- **Fix:** Isolate the power supplies. Power the servos directly from a BEC (Battery Eliminator Circuit) or separate battery, and only share the Ground wire with the MCU.

### "My I2C bus locks up entirely."
- **Cause:** The MCU was reset in the middle of an I2C read. The slave device is still waiting to output data and is holding the SDA line low. The MCU restarts, sees SDA is low, and assumes another master is using the bus.
- **Fix:** Implement a software routine on boot: if SDA is low, manually toggle the SCL pin 9 times as a GPIO output to clock out the stuck data and force the slave to release SDA.

### "My PID controller is oscillating no matter what gains I use."
- **Cause:** Loop delay or sensor noise.
- **Fix:** Ensure your PID loop is running at a strict, constant frequency (e.g., exactly 100Hz using hardware timers). Use a low-pass filter on your sensor data before feeding it to the PID.

## 5. The Software Side: Hardware-in-the-Loop (HIL)

When building complex control systems (like drone flight controllers or ROS 2 navigation), it's unsafe to test directly on real hardware.

- Use **Gazebo** or **Isaac Sim** to simulate the physical robot.
- Run your *exact* C++/Python code against the simulator. If it fails in sim, it will fail in reality. If it passes in sim, it *might* pass in reality.
