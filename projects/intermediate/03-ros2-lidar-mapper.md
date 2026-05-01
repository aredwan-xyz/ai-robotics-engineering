# Project: Autonomous 2D Mapping Robot (ROS 2)

**Difficulty:** 🟦 Intermediate  
**Skills:** ROS 2, SLAM, Linux, Python/C++  
**Estimated Time:** 20–30 hours  
**BOM Cost:** ~$150–$250

---

## What You'll Build

A differential-drive mobile robot that creates a 2D map of a room as you drive it around, using a 360° LiDAR and ROS 2 (Robot Operating System).

## Why This Matters

Simultaneous Localization and Mapping (SLAM) is the foundation of autonomous mobile robotics (like robot vacuums or warehouse robots). Building this introduces you to the ROS 2 ecosystem, coordinate frame transforms (TF2), and sensor integration.

## Hardware Required

1. **Compute:** Raspberry Pi 4 (2GB or 4GB) running Ubuntu 22.04.
2. **Sensor:** RPLidar A1M8 (~$100). This spins 360 degrees and outputs distance measurements via USB.
3. **Chassis:** A cheap 2-wheel drive robot chassis kit with DC motors and wheel encoders.
4. **Motor Controller:** L298N or similar H-Bridge motor driver.
5. **Microcontroller:** Arduino Nano or ESP32 (to read encoders and drive the motors with PWM).
6. **Power:** A USB power bank for the Raspberry Pi, and a 2S LiPo battery for the motors.

## System Architecture

The robot uses a classic distributed architecture:

1. **Low-Level (Arduino):** Runs a fast PID loop to control wheel speed. It receives `cmd_vel` (target velocity) commands via serial and sends back odometry data (how far the wheels have turned).
2. **High-Level (Raspberry Pi):** Runs ROS 2. It runs the LiDAR driver, the serial bridge to the Arduino, and the `slam_toolbox` node.
3. **Your Laptop:** Connects to the Pi over WiFi, runs RViz2 to visualize the map being built, and sends teleoperation commands (joystick or keyboard).

## Step 1: Base Controller (Arduino)

Write an Arduino sketch to act as the hardware bridge.
- **Encoders:** Use hardware interrupts to count ticks from the left and right wheel encoders.
- **PID Control:** Run a PID loop at 50Hz to maintain the commanded wheel velocities.
- **Serial Comm:** Accept target speeds (e.g., `<0.2, 0.0>` for 0.2m/s forward) and send current speeds back.

## Step 2: ROS 2 Setup & Drivers (Raspberry Pi)

Install ROS 2 Humble on the Raspberry Pi.
1. Install the `rplidar_ros` package to get laser scans published to the `/scan` topic.
2. Write a custom ROS 2 Python node (`base_controller`) that opens the Serial port, talks to your Arduino, and publishes `nav_msgs/Odometry` and TF transforms (`odom` -> `base_link`).

## Step 3: URDF & TF2

Create a Unified Robot Description Format (URDF) file.
This XML file tells ROS where the LiDAR is relative to the center of the robot. 
For example, if the LiDAR is mounted 10cm forward and 5cm high, the URDF defines a static transform from `base_link` to `laser_frame`.

## Step 4: SLAM Toolbox

ROS 2 makes SLAM incredibly easy once your data is formatted correctly.
Run the standard `slam_toolbox` package in async mapping mode.
It subscribes to:
- `/scan` (from the RPLidar)
- `/tf` (the `odom` -> `base_link` transform from your base controller)

As you drive the robot with a `teleop_twist_keyboard` node, `slam_toolbox` will stitch the laser scans together and publish an `OccupancyGrid` to the `/map` topic.

## Step 5: Visualization

On your laptop, install ROS 2 and run `rviz2`.
Add displays for:
- `RobotModel`
- `LaserScan`
- `Map`

You will see the map generating in real-time as the robot explores the room.

## Extensions

- **Navigation (Nav2):** Once you have a map, use the Nav2 stack to click a point on the map and have the robot autonomously path-plan and drive there while avoiding new obstacles.
